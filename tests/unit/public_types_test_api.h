#pragma once

#include <viewshell/application.h>

namespace viewshell {
namespace detail {

Result<NormalizedAppOptions> normalize_app_options_for_test(const AppOptions& options);

}
}
