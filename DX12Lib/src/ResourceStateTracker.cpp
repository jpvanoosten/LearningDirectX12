#include <DX12LibPCH.h>

#include <ResourceStateTracker.h>

// Initialize statics.
ResourceStateTracker::GlobalResourceState ResourceStateTracker::ms_GlobalResourceState;

ResourceStateTracker::ResourceStateTracker()
{}

ResourceStateTracker::~ResourceStateTracker()
{}
