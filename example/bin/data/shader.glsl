-- settings
mode: TRIANGLES
count: 1000

geom_count: 8
geom_mode: TRIANGLE_STRIP

-- vertex

#pragma include "Common/ShaderHelpers.glslinc"
#pragma include "Common/Noise4D.glslinc"

uniform float Time = 0;
uniform float Seed = 0.4;

void main()
{
	// int id = gl_VertexID / 2;
	// float V = step(0.5, fract(gl_VertexID * 0.33)) * 0.1;
	
	int id = gl_VertexID / 3;
	float V = mod(gl_VertexID, 3) / 3;

	vec3 P = vec3(id * 0.1 + V * 0.4, id, -id);

	P = fbmvec3(vec4(P, Time * 0.1), 5, Seed, 1) * 1500;

	gl_Position = gl_ModelViewProjectionMatrix * vec4(P, 1);
	gl_FrontColor = vec4(abs(sin(fract(P * 0.001) * TWO_PI)), 1);
	// gl_FrontColor = vec4(1);
}

-- geometry

void main()
{
	vec3 S = vec3(10, 10, 0);

	float a = length(gl_FrontColorIn[0]);

	gl_FrontColor = gl_FrontColorIn[0];
	gl_Position = gl_PositionIn[0];
	gl_Position.xyz += S * vec3(-1, -1, 0) * a;
	EmitVertex();

	gl_FrontColor = gl_FrontColorIn[0];
	gl_Position = gl_PositionIn[0];
	gl_Position.xyz += S * vec3(1, -1, 0) * a;
	EmitVertex();

	gl_FrontColor = gl_FrontColorIn[0];
	gl_Position = gl_PositionIn[0];
	gl_Position.xyz += S * vec3(-1, 1, 0) * a;
	EmitVertex();

	gl_FrontColor = gl_FrontColorIn[0];
	gl_Position = gl_PositionIn[0];
	gl_Position.xyz += S * vec3(1, 1, 0) * a;
	EmitVertex();

	EndPrimitive();
}

-- fragment

void main()
{
	gl_FragColor = gl_Color;
}
