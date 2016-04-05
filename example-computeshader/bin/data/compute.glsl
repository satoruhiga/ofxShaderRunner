-- settings
version: 430

-- compute

#pragma include "Common/ShaderHelpers.glslinc"
#pragma include "Common/Noise4D.glslinc"
#pragma include "Common/SimplexNoiseDerivatives4D.glslinc"

struct Particle{
	vec4 pos;
};

layout(std140, binding=0) buffer inOutData {
    Particle data[];
};

layout(local_size_x = 8, local_size_y = 1, local_size_z = 1) in;

uniform float Time;
uniform float TimeInc;

void main()
{
	Particle p = data[gl_GlobalInvocationID.x];

	float S = fract(Time * 0.01) * 0.01;
	vec3 V = curlNoise(p.pos.xyz * S, Time * 0.01, 2, 0.5);

	p.pos.xyz += V * TimeInc * 60;

	p.pos.w += 0.1 * TimeInc;
	if (p.pos.w > 1)
	{
		p.pos.w = rand(p.pos.xy * 0.01) * -0.2;

		p.pos.x = rand(p.pos.xy) - 0.5;
		p.pos.y = rand(p.pos.yz) - 0.5;
		p.pos.z = rand(p.pos.zx) - 0.5;
		p.pos *= 3;

		vec3 V0 = fbmvec3(vec4(0, 0, 0, Time * 0.2), 3, 2, 0.5);
		vec3 V1 = fbmvec3(vec4(0, 0, 0, (Time - TimeInc) * 0.2), 3, 2, 0.5);

		p.pos.xyz += mix(V0, V1, rand(p.pos.zx)) * 1000;
	}

	data[gl_GlobalInvocationID.x] = p;
}