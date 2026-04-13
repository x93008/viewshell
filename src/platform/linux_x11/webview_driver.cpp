#include "webview_driver.h"
#include "resource_protocol.h"

#include <webkit2/webkit2.h>
#include <gtk/gtk.h>
#include <fstream>
#include <jsc/jsc.h>

namespace viewshell {

WebviewDriver::WebviewDriver() = default;

WebviewDriver::~WebviewDriver() {
  on_close = nullptr;
  page_load_handlers_.clear();
  if (webview_ && GTK_IS_WIDGET(webview_)) {
    gtk_widget_destroy(GTK_WIDGET(webview_));
  }
}

bool WebviewDriver::is_allowed_url_scheme(std::string_view url) {
  return url.rfind("https://", 0) == 0 || url.rfind("http://", 0) == 0;
}

static ResourceProtocol* g_resource_protocol = nullptr;

static void uri_scheme_callback(WebKitURISchemeRequest* request, gpointer) {
  if (!g_resource_protocol) {
    GError* error = g_error_new_literal(G_IO_ERROR, G_IO_ERROR_NOT_FOUND,
                                         "no resource protocol");
    webkit_uri_scheme_request_finish_error(request, error);
    g_error_free(error);
    return;
  }

  const gchar* uri = webkit_uri_scheme_request_get_uri(request);
  auto result = g_resource_protocol->resolve(uri);
  if (!result) {
    GError* error = g_error_new_literal(G_IO_ERROR, G_IO_ERROR_NOT_FOUND,
                                         result.error().message.c_str());
    webkit_uri_scheme_request_finish_error(request, error);
    g_error_free(error);
    return;
  }

  GBytes* bytes = g_bytes_new(result->body.data(), result->body.size());
  GInputStream* stream = g_memory_input_stream_new_from_bytes(bytes);
  webkit_uri_scheme_request_finish(request, stream, result->body.size(),
                                    result->mime_type.c_str());
  g_object_unref(stream);
  g_bytes_unref(bytes);
}

static void on_drag_message(WebKitUserContentManager*, WebKitJavascriptResult*, gpointer user_data) {
  auto* wv = WEBKIT_WEB_VIEW(user_data);
  GtkWidget* toplevel = gtk_widget_get_toplevel(GTK_WIDGET(wv));
  GdkDisplay* display = gtk_widget_get_display(GTK_WIDGET(wv));
  GdkSeat* seat = gdk_display_get_default_seat(display);
  GdkDevice* pointer = gdk_seat_get_pointer(seat);
  gint root_x, root_y;
  gdk_device_get_position(pointer, nullptr, &root_x, &root_y);
  gtk_window_begin_move_drag(GTK_WINDOW(toplevel), 1, root_x, root_y, GDK_CURRENT_TIME);
}

static void on_webview_close(WebKitWebView* wv, gpointer) {
  gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(wv)));
}

static void on_script_message_received(WebKitUserContentManager*,
    WebKitJavascriptResult* result, gpointer user_data) {
  auto* callback = static_cast<std::function<void(std::string_view)>*>(user_data);
  if (!callback) {
    return;
  }

  JSCValue* value = webkit_javascript_result_get_js_value(result);
  if (!value || !jsc_value_is_string(value)) {
    (*callback)("{}");
    return;
  }

  char* js = jsc_value_to_string(value);
  if (!js) {
    (*callback)("{}");
    return;
  }

  (*callback)(js);
  g_free(js);
 }

Result<void> WebviewDriver::attach(NativeWindowHandle native,
                                     const WindowOptions& options) {
  if (attached_) return {};

  static bool uri_scheme_registered = false;

  auto* context = webkit_web_context_get_default();
  if (!context) {
    return tl::unexpected(Error{"engine_init_failed", "failed to get WebKit web context"});
  }

  if (!uri_scheme_registered) {
    webkit_web_context_register_uri_scheme(context, "viewshell",
        uri_scheme_callback, nullptr, nullptr);
    uri_scheme_registered = true;
  }

  user_content_manager_ = webkit_user_content_manager_new();

  webview_ = WEBKIT_WEB_VIEW(g_object_new(WEBKIT_TYPE_WEB_VIEW,
      "user-content-manager", user_content_manager_, nullptr));
  if (!webview_) {
    return tl::unexpected(Error{"engine_init_failed", "failed to create WebKitWebView"});
  }

  gtk_container_add(GTK_CONTAINER(native.gtk_window), GTK_WIDGET(webview_));
  gtk_widget_show(GTK_WIDGET(webview_));

  if (options.borderless) {
    GdkRGBA transparent = {0.0, 0.0, 0.0, 0.0};
    webkit_web_view_set_background_color(webview_, &transparent);

    webkit_user_content_manager_register_script_message_handler(
        user_content_manager_, "viewshellDrag");
    g_signal_connect(user_content_manager_,
        "script-message-received::viewshellDrag",
        G_CALLBACK(on_drag_message), webview_);
  }

  g_signal_connect(GTK_WIDGET(webview_), "close",
      G_CALLBACK(on_webview_close), nullptr);

  capabilities_.window.borderless = options.borderless;
  capabilities_.window.always_on_top = options.always_on_top;
  capabilities_.webview.devtools = true;
  capabilities_.webview.resource_protocol = true;
  capabilities_.webview.script_eval = true;

  attached_ = true;

  WebKitSettings* settings = webkit_web_view_get_settings(webview_);
  webkit_settings_set_enable_developer_extras(settings, TRUE);

  for (auto& handler : page_load_handlers_) {
    (void)handler;
  }

  for (auto& script : init_scripts_) {
    webkit_user_content_manager_add_script(user_content_manager_,
        webkit_user_script_new(script.c_str(),
            WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
            WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
            nullptr, nullptr));
  }

  return {};
}

Result<void> WebviewDriver::load_file(std::string_view entry_file) {
  if (!attached_) return tl::unexpected(Error{"invalid_state", "webview not attached"});

  auto proto = ResourceProtocol::from_entry_file(entry_file, WindowOptions{});
  if (!proto) return tl::unexpected(proto.error());

  resource_protocol_ = std::make_unique<ResourceProtocol>(std::move(*proto));
  g_resource_protocol = resource_protocol_.get();

  webkit_web_view_load_uri(webview_, resource_protocol_->entry_url().c_str());
  return {};
}

Result<void> WebviewDriver::load_url(std::string_view url) {
  if (!attached_) return tl::unexpected(Error{"invalid_state", "webview not attached"});
  if (!is_allowed_url_scheme(url)) {
    return tl::unexpected(Error{"unsupported_scheme", "only http/https allowed"});
  }
  webkit_web_view_load_uri(webview_, std::string(url).c_str());
  return {};
}

Result<void> WebviewDriver::reload() {
  if (!attached_) return tl::unexpected(Error{"invalid_state", "webview not attached"});
  webkit_web_view_reload(webview_);
  return {};
}

Result<void> WebviewDriver::evaluate_script(std::string_view script) {
  if (!attached_) return tl::unexpected(Error{"invalid_state", "webview not attached"});
  webkit_web_view_evaluate_javascript(webview_, std::string(script).c_str(),
      -1, nullptr, nullptr, nullptr, nullptr, nullptr);
  return {};
}

Result<void> WebviewDriver::add_init_script(std::string_view script) {
  init_scripts_.push_back(std::string(script));
  if (attached_ && user_content_manager_) {
    webkit_user_content_manager_add_script(user_content_manager_,
        webkit_user_script_new(std::string(script).c_str(),
            WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
            WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
            nullptr, nullptr));
  }
  return {};
}

Result<void> WebviewDriver::register_script_message_handler(
    std::string_view name,
    std::function<void(std::string_view payload)> handler) {
  if (!attached_ || !user_content_manager_) {
    return tl::unexpected(Error{"invalid_state", "webview not attached"});
  }

  auto key = std::string(name);
  script_message_handlers_[key] = std::move(handler);

  webkit_user_content_manager_register_script_message_handler(
      user_content_manager_, key.c_str());

  auto detailed_signal = std::string("script-message-received::") + key;
  g_signal_connect(user_content_manager_, detailed_signal.c_str(),
      G_CALLBACK(on_script_message_received), &script_message_handlers_[key]);

  return {};
}

Result<void> WebviewDriver::open_devtools() {
  if (!attached_) return tl::unexpected(Error{"invalid_state", "webview not attached"});
  WebKitSettings* settings = webkit_web_view_get_settings(webview_);
  webkit_settings_set_enable_developer_extras(settings, TRUE);
  WebKitWebInspector* inspector = webkit_web_view_get_inspector(webview_);
  webkit_web_inspector_show(inspector);
  return {};
}

Result<void> WebviewDriver::close_devtools() {
  if (!attached_) return tl::unexpected(Error{"invalid_state", "webview not attached"});
  WebKitWebInspector* inspector = webkit_web_view_get_inspector(webview_);
  webkit_web_inspector_close(inspector);
  return {};
}

Result<void> WebviewDriver::on_page_load(PageLoadHandler handler) {
  page_load_handlers_.push_back(std::move(handler));
  return {};
}

Result<void> WebviewDriver::set_navigation_handler(NavigationHandler handler) {
  navigation_handler_ = std::move(handler);
  return {};
}

Result<void> WebviewDriver::ensure_attached() const {
  if (!attached_) {
    return tl::unexpected(Error{"invalid_state", "webview not attached"});
  }
  return {};
}

} // namespace viewshell
