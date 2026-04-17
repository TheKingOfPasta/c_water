struct chunk_particle_idx_pair
{
    uint chunk_idx;
    uint particle_idx;
};

layout(std430, binding = BINDING_PAIRS) buffer PairsBuffer
{
    chunk_particle_idx_pair pairs[];
};
layout(std430, binding = BINDING_START_CHUNKS) buffer StartChunksBuffer
{
    uint start_chunks[];
};
