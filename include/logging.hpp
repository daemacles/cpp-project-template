#pragma once

#include <stdexcept>

#if defined( __NVCC__ )

// Speedlog doesn't support NVCC as of Aug 23:
// https://github.com/gabime/spdlog/issues/807
#define FATAL_( msg, ... ) throw std::runtime_error( "CUDA throw: " msg )
#define ERROR_( msg, ... )
#define WARN_( msg, ... )
#define INFO_( msg, ... )
#define DEBUG_( msg, ... )

#else

#if defined( _WIN32 )
#pragma warning( push, 0 )
#endif
#include <memory>

#include "stacktrace.hpp"

// So we can log things that define ostream<< operators
#include "spdlog/fmt/bundled/ostream.h"
#include "spdlog/spdlog.h"
#if defined( _WIN32 )
#pragma warning( pop )
#endif

// Returns a shared pointer to the current default log named "console"
std::shared_ptr<spdlog::logger> SpeedLog();

// Persist the logger across all translation units so that if the main thread
// exits before some other thread, the logger is not destructed prematurely.
static std::shared_ptr<spdlog::logger> local_log_ = SpeedLog();

inline std::string methodName( const std::string &prettyFunction )
{
  size_t colons = prettyFunction.find( "::" );
  size_t begin = prettyFunction.substr( 0, colons ).rfind( " " ) + 1;
  size_t end = prettyFunction.rfind( "(" ) - begin;

  return prettyFunction.substr( begin, end ) + "()";
}

#ifdef WIN32
#define __METHOD_NAME__ __FUNCTION__
#else
#define __METHOD_NAME__ methodName( __PRETTY_FUNCTION__ )
#endif  // DEBUG

inline std::string className( const std::string &prettyFunction )
{
  size_t colons = prettyFunction.find( "::" );
  if( colons == std::string::npos )
    return "::";
  size_t begin = prettyFunction.substr( 0, colons ).rfind( " " ) + 1;
  size_t end = colons - begin;

  return prettyFunction.substr( begin, end );
}

#define __CLASS_NAME__ className( __PRETTY_FUNCTION__ )

#define STRINGISE( X ) #X
#ifdef NDEBUG
#define FANCY_LOG( level, msg, ... ) SpeedLog()->level( msg, ##__VA_ARGS__ )
#else
#define FANCY_LOG( level, msg, ... )                                           \
  SpeedLog()->level( "{}:{} {} | " msg, __FILE__, __LINE__, __METHOD_NAME__,   \
                     ##__VA_ARGS__ )
#endif

#define FATAL_THROW( msg, ... )                                                \
  PrintStack();                                                                \
  throw std::runtime_error( fmt::format( msg, ##__VA_ARGS__ ) )

#ifdef WIN32
#define DEBUG_BREAK_()                                                         \
  do                                                                           \
  {                                                                            \
    if( IsDebuggerPresent() )                                                  \
    {                                                                          \
      DebugBreak();                                                            \
    }                                                                          \
  } while( 0 )
#define FATAL_( msg, ... )                                                     \
  do                                                                           \
  {                                                                            \
    SpeedLog()->error( "{}:{} {} | " msg, __FILE__, __LINE__, __METHOD_NAME__, \
                       ##__VA_ARGS__ );                                        \
    SpeedLog()->flush();                                                       \
    PrintStack();                                                              \
    DEBUG_BREAK_();                                                            \
    throw std::runtime_error( fmt::format( msg, ##__VA_ARGS__ ) );             \
  } while( 0 )
#define ERROR_( msg, ... )                                                     \
  do                                                                           \
  {                                                                            \
    SpeedLog()->error( "{}:{} {} | " msg, __FILE__, __LINE__, __METHOD_NAME__, \
                       ##__VA_ARGS__ );                                        \
    SpeedLog()->flush();                                                       \
    PrintStack();                                                              \
    DEBUG_BREAK_();                                                        \
  } while( 0 )
#define WARN_( msg, ... )                                                      \
  SpeedLog()->warn( "{}:{} {} | " msg, __FILE__, __LINE__, __METHOD_NAME__,    \
                    ##__VA_ARGS__ )
#define INFO_( msg, ... )                                                      \
  SpeedLog()->info( "{}:{} {} | " msg, __FILE__, __LINE__, __METHOD_NAME__,    \
                    ##__VA_ARGS__ )

#ifdef NDEBUG
#define DEBUG_( ... )
#else
#define DEBUG_( msg, ... )                                                     \
  SpeedLog()->debug( "{}:{} {} | " msg, __FILE__, __LINE__, __METHOD_NAME__,   \
                     ##__VA_ARGS__ )
#endif

#else  // WIN32

#define FATAL_( ... )                                                          \
  FANCY_LOG( error, __VA_ARGS__ );                                             \
  SpeedLog()->flush();                                                         \
  FATAL_THROW( __VA_ARGS__ )
#define ERROR_( ... )                                                          \
  FANCY_LOG( error, __VA_ARGS__ );                                             \
  SpeedLog()->flush();                                                         \
  PrintStack()
#define WARN_( ... ) FANCY_LOG( warn, __VA_ARGS__ )
#define INFO_( ... ) FANCY_LOG( info, __VA_ARGS__ )

#ifdef NDEBUG
#define DEBUG_( ... )
#else
#define DEBUG_( ... ) FANCY_LOG( debug, __VA_ARGS__ )
#endif

#endif  // WIN32

#endif  // if defined __NVCC__

#define FATAL_IF_( COND, msg, ... )                                            \
  if( COND )                                                                   \
  {                                                                            \
    FATAL_( msg, __VA_ARGS__ );                                                \
  }
#define WARN_IF_( COND, msg, ... )                                             \
  if( COND )                                                                   \
  {                                                                            \
    WARN_( msg, __VA_ARGS__ );                                                 \
  }
