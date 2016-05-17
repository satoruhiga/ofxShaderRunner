 	-- settings
mode: POINTS
count: 8000

geom_count: 4
geom_mode: TRIANGLE_STRIP

-- vertex

#pragma include "Common/Common.glslinc"

#pragma glslify: snoise4 = require(glsl-noise/simplex/4d)

uniform float Time = 0;

#define v4(fn, v) vec3(fn(v), \
	fn(v + vec4(134, 235, 523, 123)), \
	fn(v + vec4(948, 1245, 123, -024)))

void main()
{
	vec3 P = gl_Vertex.xyz;
	P += v4(snoise4, vec4(P * 1, Time)) * 0.2;
	gl_Position = gl_ModelViewProjectionMatrix * vec4(P, 1);
}

-- fragment

void main()
{
	gl_FragColor = vec4(1);
}
