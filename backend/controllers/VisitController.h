#pragma once

#if __has_include("../include/httplib.h")
#  include "../include/httplib.h"
#elif __has_include("include/httplib.h")
#  include "include/httplib.h"
#else
#  include <httplib.h>
#endif

/**
 * @brief 随访管理控制器
 */
class VisitController {
public:
    static void registerRoutes(httplib::Server& svr);
};
