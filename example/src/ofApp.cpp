#include "ofMain.h"

#include "ofxShaderRunner.h"

class ofApp : public ofBaseApp
{
public:

	ofEasyCam cam;
	
	ofxShaderRunner S;

	void setup()
	{
		ofSetFrameRate(60);
		ofSetVerticalSync(true);
		ofBackground(0);
		
		S.load("shader.glsl");
	}

	void update()
	{
		ofSetWindowTitle(ofToString(ofGetFrameRate()));
	}

	void draw()
	{
		ofEnableDepthTest();
		glPointSize(4);
		
		cam.begin();
		{
			S.draw();
		}
		cam.end();
	}

	void keyPressed(int key)
	{
	}

	void keyReleased(int key)
	{
	}

	void mouseMoved(int x, int y)
	{
	}

	void mouseDragged(int x, int y, int button)
	{
	}

	void mousePressed(int x, int y, int button)
	{
	}

	void mouseReleased(int x, int y, int button)
	{
	}

	void windowResized(int w, int h)
	{
	}
};


int main(int argc, const char** argv)
{
	ofSetupOpenGL(640, 480, OF_WINDOW);
	ofRunApp(new ofApp);
	return 0;
}