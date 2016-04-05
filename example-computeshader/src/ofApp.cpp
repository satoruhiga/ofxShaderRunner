#include "ofMain.h"

#include "ofxShaderRunner.h"

class ofApp : public ofBaseApp
{
public:

	ofEasyCam cam;
	ofxShaderRunner shader;

	ofVbo vbo;
	ofBufferObject buffer;

	int N = 100000;

	struct Particle {
		ofVec4f pos;
	};

	void setup()
	{
		ofSetFrameRate(0);
		ofSetVerticalSync(false);
		ofBackground(0);

		shader.load("compute.glsl");

		vector<Particle> data(N);
		for (int i = 0; i < N; i++)
		{
			float R = 1000;
			data[i].pos.x = ofRandom(-R, R);
			data[i].pos.y = ofRandom(-R, R);
			data[i].pos.z = ofRandom(-R, R);
			data[i].pos.w = ofRandom(1);
		}

		buffer.allocate(data, GL_DYNAMIC_DRAW);
		buffer.bindBase(GL_SHADER_STORAGE_BUFFER, 0);

		vbo.setVertexBuffer(buffer, 3, sizeof(Particle), 0);
	}

	void update()
	{
		ofSetWindowTitle(ofToString(ofGetFrameRate()));

		shader.begin();
		shader.dispatchCompute(N, 1, 1);
		shader.end();
	}

	
	void draw()
	{
		ofEnableBlendMode(OF_BLENDMODE_ADD);

		cam.begin();
		ofDrawAxis(10);
		ofSetColor(255, 32);
		vbo.draw(GL_POINTS, 0, N);
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
	ofGLWindowSettings settings;
	settings.setGLVersion(4, 3);
	settings.width = 1280;
	settings.height = 720;
	ofCreateWindow(settings);
	ofRunApp(new ofApp);
	return 0;
}