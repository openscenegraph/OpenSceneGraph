uniform float inversePeriod;
uniform vec4 particleColour;
uniform float particleSize;

uniform float osg_FrameTime;

varying vec4 colour;

void main(void)
{
    float offset = gl_Vertex.z;
    float startTime = gl_MultiTexCoord1.x;

    vec4 v_current = gl_Vertex;
    v_current.z = fract( (osg_FrameTime - startTime)*inversePeriod - offset);
   
    colour = particleColour;

    gl_Position = gl_ModelViewProjectionMatrix * v_current;

    float pointSize = abs(1280.0*particleSize / gl_Position.w);

    //gl_PointSize = max(ceil(pointSize),2);
    gl_PointSize = ceil(pointSize);
    
    colour.a = 0.05+(pointSize*pointSize)/(gl_PointSize*gl_PointSize);
}
