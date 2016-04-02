-- settings
mode: POINTS
count: 8000

geom_count: 4
geom_mode: TRIANGLE_STRIP

-- vertex

#pragma include "Common/Common.glslinc"
#pragma include "Common/ShaderHelpers.glslinc"
#pragma include "Common/Noise4D.glslinc"

uniform float Time = 0;

void main()
{
	vec3 G = grid(20, 20, 20);
	vec3 V = snoisevec3(vec4(Time * 0.1, 0, 0, 0)) + vec3(0, 0, Time * 0.1);
	vec3 C = G + V;
	G = fract(C);

	vec3 P = G - 0.5;
	P = (rotationMatrix(vec3(0, 1, 0), Time * 0.5) * vec4(P, 1)).xyz;

	P *= 2500;

	P += fbmvec3(vec4(C, Time * 0.5), 5, 1, 1) * 500 * map(sin(Time * 0.1), -1, 1, 0, 1);

	gl_Position = gl_ModelViewProjectionMatrix * vec4(P, 1);
	gl_FrontColor = vec4(abs(sin(C * 10)), 1);
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
