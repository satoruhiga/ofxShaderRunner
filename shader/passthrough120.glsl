-- settings
version: 120
mode: POINTS
count: 1

geom_count: 1
geom_mode: POINTS

-- vertex

uniform float Time = 0;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_FrontColor = gl_Color;
}

-- geometry

void main()
{
	gl_FrontColor = gl_FrontColorIn[0];
	gl_Position = gl_PositionIn[0];
	EmitVertex();

	EndPrimitive();
}

-- fragment

void main()
{
	gl_FragColor = gl_Color;
}
