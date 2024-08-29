#version 450 

#define VertexData \
	_VertexData { \
		noperspective float m_edgeDistance; \
		noperspective float m_size; \
		smooth vec4 m_color; \
	}

#define kAntialiasing 2.0

in VertexData vData;
	
layout(location=0) out vec4 fResult;

void main() 
{
    fResult = vData.m_color;

}