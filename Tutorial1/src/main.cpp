#include <Tutorial1PCH.h>
#include <DX12Game.h>

using namespace DirectX12Template;

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^ args)
{
    DX12Game game(800, 600, L"Hello World!");

    auto viewSource = ref new DX12FrameworkViewSource(reinterpret_cast<size_t>(&game));

    Windows::ApplicationModel::Core::CoreApplication::Run(viewSource);

    return 0;
}