struct chunk_particle_idx_pair
{
    int chunk_idx;
    int particle_idx;
};

layout(std430, binding = BINDING_PAIRS) buffer PairsBuffer
{
    chunk_particle_idx_pair pairs[];
};
layout(std430, binding = BINDING_START_CHUNKS) buffer StartChunksBuffer
{
    int start_chunks[];
};
layout(std430, binding = BINDING_END_CHUNKS) buffer EndChunksBuffer
{
    int end_chunks[];
};
