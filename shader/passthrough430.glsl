-- settings
version: 430
mode: POINTS
count: 1

-- vertex

#pragma include "Common/ofCommon.glslinc"

uniform float Time = 0;

out VS {
	vec4 color;
} outData;

void main()
{
	gl_Position = modelViewProjectionMatrix * position;
	outData.color = (usingColors > 0) ? color : globalColor;
}

-- geometry

layout(points) in;
layout(points, max_vertices = 1) out;

in VS {
	vec4 color;
} inData[];

out GS {
	vec4 color;
} outData;

void main()
{
	gl_Position = gl_in[0].gl_Position;
	outData.color = inData[0].color;
	EmitVertex();

	EndPrimitive();
}

-- fragment

// in VS {
// 	vec4 color;
// } inData;

in GS {
	vec4 color;
} inData;

out vec4 outColor;

void main()
{
	outColor = inData.color;
}
