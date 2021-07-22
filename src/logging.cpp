#if defined( _WIN32 )
#pragma warning(push, 0)
#endif

#include <cstdlib>

#include <filesystem>
#include <iostream>
#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>

#if defined( _WIN32 )
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/wincolor_sink.h>
#pragma warning(pop)
#endif

#include "logging.hpp"
#include "pystring.h"

std::shared_ptr<spdlog::logger> SpeedLog()
{
  static std::shared_ptr<spdlog::logger> log = []() {
    auto cur_log = spdlog::get( "console" );
    if( !cur_log )
    {
      // Look up name of current process
      TCHAR proc_filename[2048];
      GetModuleFileName( nullptr, proc_filename, 2048 );
      std::filesystem::path proc_path{ proc_filename };

      std::string logname =
          fmt::format( "logfile-{}.log", proc_path.stem().string() );

      std::vector<spdlog::sink_ptr> sinks = {
#ifdef _WIN32
        std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>(),
        std::make_shared<spdlog::sinks::msvc_sink_mt>(),
#else
        std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>(),
#endif
        std::make_shared<spdlog::sinks::daily_file_sink_mt>( logname, 23, 59 )
      };

      // By default log to stdout
      cur_log = std::make_shared<spdlog::logger>( "console", begin( sinks ),
                                                  end( sinks ) );
      // spdlog::stdout_color_mt("console");
      cur_log->set_pattern( "[%Y%m%d %H:%M:%S.%e] %l | %v" );
      cur_log->info( "Logging to default console and logfile.log" );

      char *log_level_raw = nullptr;
      bool get_env_success = false;
#if defined( _WIN32 )
      size_t sz = 0;
      get_env_success = ( _dupenv_s( &log_level_raw, &sz, "LOGLEVEL" ) == 0 &&
                          log_level_raw != nullptr );
#else
      log_level_raw = getenv( "LOGLEVEL" );
      get_env_success = log_level_raw;
#endif
      if( get_env_success )
      {
        std::string log_level = pystring::lower( std::string{ log_level_raw } );
        std::cout << "Log level is '" << log_level << "'\n";
        if( log_level == "debug" )
        {
          cur_log->set_level( spdlog::level::debug );
        }
        else if( log_level == "info" )
        {
          cur_log->set_level( spdlog::level::info );
        }
        else if( log_level == "warn" )
        {
          cur_log->set_level( spdlog::level::warn );
        }
        else if( log_level == "error" )
        {
          cur_log->set_level( spdlog::level::err );
        }
        else
        {
          WARN_( "'{}' is an unknown log level. Please use one of ``debug'', "
                 "``info'', ``warn'', or ``error''.",
                 log_level );
        }
      }
    }
    return cur_log;
  }();
  log->flush_on( spdlog::level::err );
  return log;
}
