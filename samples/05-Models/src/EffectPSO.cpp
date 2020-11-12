#include <EffectPSO.h>

#include <dx12lib/CommandList.h>

EffectPSO::EffectPSO(std::shared_ptr<dx12lib::Device> device)
: m_Device(device)
{}
