struct VoxelNode {
	uint albedo;
    uint emissive;
    uint normal;
    int header;
};

uint packRGBA8(vec4 value) {
    // Clamp to ensure values stay within safely packable [0.0, 1.0] range
    uvec4 unnormalized = uvec4(saturate(value) * 255.0);
    
    // Shift channels into their respective byte positions
    return (unnormalized.r << 24u) | 
           (unnormalized.g << 16u) | 
           (unnormalized.b << 8u)  | 
           unnormalized.a;
}
vec4 unpackRGBA8(uint packedValue) {
    uvec4 unpackedChannels;
    unpackedChannels.r = (packedValue >> 24u) & 0xFFu;
    unpackedChannels.g = (packedValue >> 16u) & 0xFFu;
    unpackedChannels.b = (packedValue >> 8u)  & 0xFFu;
    unpackedChannels.a = packedValue & 0xFFu;
    
    // Convert back to [0.0, 1.0] float range
    return vec4(unpackedChannels) / 255.0;
}

// Converts a float to an 11-bit float (5 exponent, 6 mantissa)
uint floatTo11Bit(float val) {
    if (val <= 0.0) return 0u;
    
    uint f = floatBitsToUint(val);
    uint sign = (f >> 31u) & 1u;
    uint exp = (f >> 23u) & 0xFFu;
    uint mant = f & 0x7FFFFFu;

    // Handle overflow / Inf / NaN
    if (exp >= 0x1Fu + 112u) {
        return (0x1Fu << 6u) | (mant != 0u ? 0x3Fu : 0u);
    }
    // Handle underflow / Denormal
    if (exp <= 112u) {
        return 0u; 
    }

    // Re-bias exponent (-127 + 15)
    uint newExp = exp - 112u;
    uint newMant = mant >> 17u; // Keep top 6 bits

    return (newExp << 6u) | newMant;
}
// Converts a float to a 10-bit float (5 exponent, 5 mantissa)
uint floatTo10Bit(float val) {
    if (val <= 0.0) return 0u;
    
    uint f = floatBitsToUint(val);
    uint exp = (f >> 23u) & 0xFFu;
    uint mant = f & 0x7FFFFFu;

    if (exp >= 0x1Fu + 112u) {
        return (0x1Fu << 5u) | (mant != 0u ? 0x1Fu : 0u);
    }
    if (exp <= 112u) {
        return 0u;
    }

    uint newExp = exp - 112u;
    uint newMant = mant >> 18u; // Keep top 5 bits

    return (newExp << 5u) | newMant;
}
// Converts an 11-bit float back to a 32-bit float
float bit11ToFloat(uint val) {
    uint exp = (val >> 6u) & 0x1Fu;
    uint mant = val & 0x3Fu;

    if (exp == 0u) {
        if (mant == 0u) return 0.0;
        // Denormalized
        return ldexp(float(mant), -14 - 6);
    } else if (exp == 0x1Fu) {
        return uintBitsToFloat(0x7F800000u | (mant << 17u));
    }

    uint newExp = exp + 112u;
    uint newMant = mant << 17u;
    return uintBitsToFloat((newExp << 23u) | newMant);
}
// Converts a 10-bit float back to a 32-bit float
float bit10ToFloat(uint val) {
    uint exp = (val >> 5u) & 0x1Fu;
    uint mant = val & 0x1Fu;

    if (exp == 0u) {
        if (mant == 0u) return 0.0;
        return ldexp(float(mant), -14 - 5);
    } else if (exp == 0x1Fu) {
        return uintBitsToFloat(0x7F800000u | (mant << 18u));
    }

    uint newExp = exp + 112u;
    uint newMant = mant << 18u;
    return uintBitsToFloat((newExp << 23u) | newMant);
}
// PACK: Compresses vec3 RGB HDR into a single uint
uint packR11G11B10(vec3 rgb) {
    uint r = floatTo11Bit(rgb.r);
    uint g = floatTo11Bit(rgb.g);
    uint b = floatTo10Bit(rgb.b);

    // Layout: [ Blue (10 bits) | Green (11 bits) | Red (11 bits) ]
    return (b << 22u) | (g << 11u) | r;
}
// UNPACK: Extracts vec3 RGB HDR from a single uint
vec3 unpackR11G11B10(uint packedVal) {
    uint r = packedVal & 0x7FFu;         // Lower 11 bits
    uint g = (packedVal >> 11u) & 0x7FFu; // Middle 11 bits
    uint b = (packedVal >> 22u) & 0x3FFu; // Upper 10 bits

    return vec3(
        bit11ToFloat(r),
        bit11ToFloat(g),
        bit10ToFloat(b)
    );
}

// Helper for octahedral packing
vec2 octEncode(vec3 n) {
    vec2 p = n.xy * (1.0 / (abs(n.x) + abs(n.y) + abs(n.z)));
    return (n.z <= 0.0) ? ((1.0 - abs(p.yx)) * sign(p)) : p;
}
uint packNormal(vec3 normal) {
    vec2 encoded = octEncode(normalize(normal));
    // Remap from [-1, 1] to [0, 1]
    encoded = encoded * 0.5 + 0.5; 
    
    uint x = uint(encoded.x * 65535.0);
    uint y = uint(encoded.y * 65535.0);
    return (x << 16u) | y;
}
vec3 unpackNormal(uint packedVal) {
    vec2 encoded;
    encoded.x = float((packedVal >> 16u) & 0xFFFFu) / 65535.0;
    encoded.y = float(packedVal & 0xFFFFu) / 65535.0;
    encoded = encoded * 2.0 - 1.0;
    
    // Decode octahedron
    vec3 n = vec3(encoded.x, encoded.y, 1.0 - abs(encoded.x) - abs(encoded.y));
    float t = max(-n.z, 0.0);
    n.x += n.x >= 0.0 ? -t : t;
    n.y += n.y >= 0.0 ? -t : t;
    return normalize(n);
}

ivec3 getVoxelIndex(vec3 worldPos, vec3 volumeWorldMin, vec3 volumeWorldMax, int volumeDimension) {
    vec3 relativePos = (worldPos - volumeWorldMin) / (volumeWorldMax - volumeWorldMin);
    return clamp(ivec3(floor(relativePos * float(volumeDimension))), ivec3(0), ivec3(volumeDimension-1));
}

uint generateMorton3D(ivec3 v) {
    uint x = uint(v.x);
    uint y = uint(v.y);
    uint z = uint(v.z);
    x = (x | (x << 16)) & 0x030000FF;
    x = (x | (x << 8)) & 0x0300F00F;
    x = (x | (x << 4)) & 0x030C30C3;
    x = (x | (x << 2)) & 0x09249249;
    y = (y | (y << 16)) & 0x030000FF;
    y = (y | (y << 8)) & 0x0300F00F;
    y = (y | (y << 4)) & 0x030C30C3;
    y = (y | (y << 2)) & 0x09249249;
    z = (z | (z << 16)) & 0x030000FF;
    z = (z | (z << 8)) & 0x0300F00F;
    z = (z | (z << 4)) & 0x030C30C3;
    z = (z | (z << 2)) & 0x09249249;
    return x | (y << 1) | (z << 2);
}

int getFlattenedIndex(ivec3 voxelIndex, int volumeDimension) {
    // Morton
    return int(generateMorton3D(voxelIndex));

    // Spatial
    return 
        voxelIndex.x 
        + voxelIndex.y * volumeDimension
        + voxelIndex.z * volumeDimension * volumeDimension;
}

