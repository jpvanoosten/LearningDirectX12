#include <EnginePCH.h>

#include <LogStream.h>

using namespace Core;

LogStreamFile::LogStreamFile( const std::wstring& fileName )
    : m_ofs( fileName, std::wofstream::out )
{}

LogStreamFile::~LogStreamFile()
{
    if ( m_ofs.is_open() )
    {
        m_ofs.close();
    }
}

void LogStreamFile::Write( LogLevel level, const std::wstring& message )
{
    scoped_lock lock( m_FileMutex );
    if ( m_ofs.is_open() )
    {
        m_ofs << message;
        m_ofs.flush();
    }
}

static const int MAX_CONSOLE_LINES = 500;

/**
* Redirect standard output to a console window.
* Original source: http://dslweb.nwnexus.com/~ast/dload/guicon.htm
* With modifications from: http://stackoverflow.com/questions/311955/redirecting-cout-to-a-console-in-windows
* @date: December 28, 2015
*/
LogStreamConsole::LogStreamConsole()
{
    // Allocate a console. 
    if ( AllocConsole() )
    {
        HANDLE lStdHandle = GetStdHandle( STD_OUTPUT_HANDLE );

        // Increase screen buffer to allow more lines of text than the default.
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        GetConsoleScreenBufferInfo( lStdHandle, &consoleInfo );
        consoleInfo.dwSize.Y = MAX_CONSOLE_LINES;
        SetConsoleScreenBufferSize( lStdHandle, consoleInfo.dwSize );
        SetConsoleCursorPosition( lStdHandle, { 0, 0 } );

        // Redirect unbuffered STDOUT to the console.
        int hConHandle = _open_osfhandle( (intptr_t)lStdHandle, _O_TEXT );
        FILE* fp = _fdopen( hConHandle, "w" );
        freopen_s( &fp, "CONOUT$", "w", stdout );
        setvbuf( stdout, nullptr, _IONBF, 0 );

        // Redirect unbuffered STDIN to the console.
        lStdHandle = GetStdHandle( STD_INPUT_HANDLE );
        hConHandle = _open_osfhandle( (intptr_t)lStdHandle, _O_TEXT );
        fp = _fdopen( hConHandle, "r" );
        freopen_s( &fp, "CONIN$", "r", stdin );
        setvbuf( stdin, nullptr, _IONBF, 0 );

        // Redirect unbuffered STDERR to the console.
        lStdHandle = GetStdHandle( STD_ERROR_HANDLE );
        hConHandle = _open_osfhandle( (intptr_t)lStdHandle, _O_TEXT );
        fp = _fdopen( hConHandle, "w" );
        freopen_s( &fp, "CONOUT$", "w", stderr );
        setvbuf( stderr, nullptr, _IONBF, 0 );

        //Clear the error state for each of the C++ standard stream objects. We need to do this, as
        //attempts to access the standard streams before they refer to a valid target will cause the
        //iostream objects to enter an error state. In versions of Visual Studio after 2005, this seems
        //to always occur during startup regardless of whether anything has been read from or written to
        //the console or not.
        std::wcout.clear();
        std::cout.clear();
        std::wcerr.clear();
        std::cerr.clear();
        std::wcin.clear();
        std::cin.clear();
    }
}

void LogStreamConsole::Write( LogLevel level, const std::wstring& message )
{
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    HANDLE hConOut = GetStdHandle( STD_OUTPUT_HANDLE );
    // Get the console info so we can restore it after.
    GetConsoleScreenBufferInfo( hConOut, &consoleInfo );

    switch ( level )
    {
    case LogLevel::Info:
        SetConsoleTextAttribute( hConOut, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::wcout << message;
        break;
    case LogLevel::Warning:
        SetConsoleTextAttribute( hConOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY );
        std::wcout << message;
        break;
    case LogLevel::Error:
        SetConsoleTextAttribute( hConOut, FOREGROUND_RED | FOREGROUND_INTENSITY );
        std::wcerr << message;
        break;
    default:
        std::wcout << message;
        break;
    }

    // Restore console attributes.
    SetConsoleTextAttribute( hConOut, consoleInfo.wAttributes );
}

void LogStreamVS::Write( LogLevel level, const std::wstring& message )
{
    OutputDebugStringW( message.c_str() );
}

LogStreamMessageBox::LogStreamMessageBox( LogLevel levels )
    : m_LogLevels( levels )
{}

void LogStreamMessageBox::Write( LogLevel level, const std::wstring& message )
{
    std::wstring caption;
    UINT uType = 0;

    switch ( level )
    {
    case LogLevel::Info:
        caption = L"Information";
        uType = MB_ICONINFORMATION;
        break;
    case LogLevel::Warning:
        caption = L"Warning";
        uType = MB_ICONWARNING;
        break;
    case LogLevel::Error:
        caption = L"Error";
        uType = MB_ICONERROR;
        break;
    }

    if ( ( m_LogLevels & level ) != 0 )
    {
        MessageBoxW( NULL, message.c_str(), caption.c_str(), uType );
    }
}