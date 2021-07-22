#include "stacktrace.hpp"
#include "logging.hpp"

#ifdef WIN32
#include <windows.h>
#include <DbgHelp.h>

static bool s_init;

//---------------------------------------------------------------------------
void PrintStack() { SpeedLog()->info( "Stack Trace:\n" + GetStackTrace() ); }

//---------------------------------------------------------------------------
std::vector<void *> GetStackFunctionPointers()
{
  auto currentProc = GetCurrentProcess();
  if( !s_init )
  {
    s_init = true;
    bool init_succeeded = SymInitialize( currentProc, nullptr, true );
    if( !init_succeeded )
    {
      WARN_( "Failed to initialize debug symbol handling." );
    }

    SymSetOptions( SYMOPT_LOAD_LINES );
  }

  static constexpr size_t MAX_FRAMES = 32;
  std::vector<void *> frame_pointers{ MAX_FRAMES, 0 };

  const ULONG frames_to_skip = 0;
  ULONG backtrace_hash = 0;

  const USHORT n_frame =
      CaptureStackBackTrace( frames_to_skip, ( DWORD )frame_pointers.size(),
                             frame_pointers.data(), &backtrace_hash );

  frame_pointers.resize( n_frame );

  return frame_pointers;
}

//---------------------------------------------------------------------------
std::string GetStackTrace()
{
  auto currentProc = GetCurrentProcess();
  std::vector<void *> frame_pointers = GetStackFunctionPointers();
  std::string all_stack_trace_lines = "";
  uint32_t displayed_frame_index = 0;
  for( USHORT iFrame = 0; iFrame < frame_pointers.size(); ++iFrame )
  {
    const uint32_t max_name_length = 1024;
    uint8_t
        buffer[sizeof( SYMBOL_INFO ) + max_name_length + sizeof( ULONG ) - 1];
    SYMBOL_INFO *sym_info_ptr = (SYMBOL_INFO *)buffer;
    sym_info_ptr->SizeOfStruct = sizeof( SYMBOL_INFO );
    sym_info_ptr->MaxNameLen = 1024;
    uint64_t displacement = 0;
    auto result = SymFromAddr( currentProc, (DWORD64)frame_pointers[iFrame],
                               &displacement, sym_info_ptr );
    const char *frame_text = "<Unknown>";
    bool is_entry_frame = false;
    if( result )
    {
      frame_text = sym_info_ptr->Name;
      if( strcmp( frame_text, "GetStackTrace" ) == 0 ||
          strcmp( frame_text, "PrintStack" ) == 0 )
      {
        continue;
      }

      is_entry_frame = strcmp( frame_text, "main" ) == 0;

      IMAGEHLP_LINE64 line;
      line.SizeOfStruct = sizeof( IMAGEHLP_LINE64 );
      DWORD displacement32 = 0;
      if( SymGetLineFromAddr( currentProc, (DWORD64)frame_pointers[iFrame],
                              &displacement32, &line ) )
      {
        std::string full_frame_line = std::string( frame_text ) + " @ " +
                                      std::string( line.FileName ) + ":" +
                                      std::to_string( line.LineNumber );
        frame_text = full_frame_line.c_str();
      }
    }

    std::string line_str =
        fmt::format( "[{:03}] {} | {}\n", displayed_frame_index,
                     frame_pointers[iFrame], frame_text );

    all_stack_trace_lines += line_str;

    displayed_frame_index += 1;
    if( is_entry_frame )
    {
      break;
    }
  }

  return all_stack_trace_lines;
}

#else

#include <backward.hpp>

void PrintStack()
{
  backward::StackTrace st;
  st.load_here( 32 );
  backward::Printer p;
  p.print( st );
  return;
}

#endif
