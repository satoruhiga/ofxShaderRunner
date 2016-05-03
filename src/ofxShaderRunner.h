#pragma once

#include "ofMain.h"

class ofxShaderRunner : public ofShader
{
public:
	
	~ofxShaderRunner()
	{
		disableAutoReload();
	}
	
	bool load(const string& glsl_path)
	{
		unload();
		this->glsl_path = glsl_path;

		// set dummy vertex
		ofVec3f v(0);
		vbo.setVertexData(&v, 1, GL_STATIC_DRAW);

		if (ofFile::doesFileExist(glsl_path) == false)
			return false;
		
		ofBuffer buf = ofBufferFromFile(glsl_path);
		
		string tag;
		string code;
		
		codes.clear();
		
		int line_no = 0;
		
		for (auto line : buf.getLines())
		{
			if (line.substr(0, 2) == "--")
			{
				if (tag.empty() == false)
				{
					setCode(tag, code);
				}
				
				stringstream ss;
				ss << line;
				ss >> tag >> tag;
				
				code.clear();
				
				code += "#version " + ofToString(version) + "\n";
				code += "#line " + ofToString(line_no) + "\n";

				if (tag == "vertex")
				{
					if (ofGLCheckExtension("GL_EXT_gpu_shader4"))
						code += "#extension GL_EXT_gpu_shader4 : require\n"; // for gl_VertexID
				}
				else if (tag == "geometry")
				{
					if (ofGLCheckExtension("GL_EXT_geometry_shader4"))
						code += "#extension GL_EXT_geometry_shader4 : enable\n"; // for geometry shader
					else throw;
				}
				else if (tag == "compute")
				{
					if (ofGLCheckExtension("GL_ARB_compute_shader"))
						code += "#extension GL_ARB_compute_shader : enable\n";
					else throw;
				}
			}
			else
			{
				code += line + "\n";
			}
			
			line_no++;
		}
		
		if (tag.empty() == false)
		{
			setCode(tag, code);
		}
		
		ofShader::setGeometryOutputCount(geom_count);
		ofShader::setGeometryInputType(mode);
		ofShader::setGeometryOutputType(geom_mode);
		
		bindDefaults();
		
		GLuint clearErrors = glGetError();

		auto rc = linkProgram();
		glGetProgramiv(getProgram(), GL_LINK_STATUS, &status);
		
		last_check_time = ofGetElapsedTimeMillis();
		last_modified_time = std::filesystem::last_write_time(ofToDataPath(this->glsl_path));
		
		enableAutoReload();
		
		return rc;
	}
	
	void setVersion(int version) { this->version = version; }
	
	void draw()
	{
		if (status && alpha > 0)
		{
			ofxShaderRunner::begin();
			vbo.draw(mode, 0, count);
			ofxShaderRunner::end();
		}
	}
	
	void begin()
	{
		ofShader::begin();
		setUniform1f("Time", ofGetElapsedTimef());
		setUniform1f("TimeInc", ofGetLastFrameTime());
		setUniform1f("Alpha", alpha);
	}

	void end()
	{
		ofShader::end();
	}

	void setAlpha(float v) { alpha = v; }
	float getAlpha() const { return alpha; }
	
	void enableAutoReload()
	{
		if (autoreload) return;
		autoreload = true;
		
		ofAddListener(ofEvents().update, this, &ofxShaderRunner::_update);
	}
	
	void disableAutoReload()
	{
		if (!autoreload) return;
		autoreload = false;
		
		ofRemoveListener(ofEvents().update, this, &ofxShaderRunner::_update);
	}
	
	ofVbo& getVbo() { return vbo; }

protected:
	
	int version = 120;
	string glsl_path;
	map<string, string> codes;
	
	int count = 1;
	GLenum mode = GL_POINTS;
	int geom_count = 16;
	GLenum geom_mode = GL_POINTS;
	
	GLint status;
	
	int last_check_time = 0;
	bool autoreload = false;
	
	std::time_t last_modified_time;
	
	float alpha = 1;

	ofVbo vbo;
	
	void setCode(const string& tag, const string& code)
	{
		if (tag == "settings")
		{
			auto buf = ofBuffer(code);
			
			for (auto line : buf.getLines())
			{
				auto e = ofSplitString(line, ":", false, true);
				if (e.size() != 2)
					continue;
				
				if (e[0] == "count")
				{
					count = std::stoi(e[1]);
				}
				else if (e[0] == "mode")
				{
					if (e[1] == "POINTS")
						mode = GL_POINTS;
					else if (e[1] == "LINES")
						mode = GL_LINES;
					else if (e[1] == "LINE_STRIP")
						mode = GL_LINE_STRIP;
					else if (e[1] == "LINE_LOOP")
						mode = GL_LINE_LOOP;
					else if (e[1] == "TRIANGLES")
						mode = GL_TRIANGLES;
					else if (e[1] == "TRIANGLE_STRIP")
						mode = GL_TRIANGLE_STRIP;
					else if (e[1] == "TRIANGLE_FAN")
						mode = GL_TRIANGLE_FAN;
					else throw;
				}
				else if (e[0] == "version")
				{
					version = std::stoi(e[1]);
				}
				else if (e[0] == "geom_count")
				{
					geom_count = std::stoi(e[1]);
				}
				else if (e[0] == "geom_mode")
				{
					if (e[1] == "POINTS")
						geom_mode = GL_POINTS;
					else if (e[1] == "LINE_STRIP")
						geom_mode = GL_LINE_STRIP;
					else if (e[1] == "TRIANGLE_STRIP")
						geom_mode = GL_TRIANGLE_STRIP;
					else throw;
				}
			}
		}
		else if (tag == "vertex")
		{
			setupShaderFromSource(GL_VERTEX_SHADER, code);
		}
		else if (tag == "fragment")
		{
			setupShaderFromSource(GL_FRAGMENT_SHADER, code);
		}
		else if (tag == "geometry")
		{
			setupShaderFromSource(GL_GEOMETRY_SHADER, code);
		}
		else if (tag == "compute")
		{
			setupShaderFromSource(GL_COMPUTE_SHADER, code);
		}
		
		codes[tag] = code;
	}
	
	void _update(ofEventArgs &e)
	{
		auto D = ofGetElapsedTimeMillis() - last_check_time;
		if (D > 300)
		{
			last_check_time = ofGetElapsedTimeMillis();
			
			auto T = std::filesystem::last_write_time(ofToDataPath(glsl_path));
			if (last_modified_time != T)
			{
				load(glsl_path);
			}
		}
	}

public:

	template <typename T>
	struct PingPong {
		T buffer[2];

		T* front;
		T* back;

		std::function<void(T& front, T& back)> update;

		PingPong()
			: front(&buffer[0])
			, back(&buffer[1])
		{}

		void setup(
			std::function<void(T& buf)> init_fn,
			std::function<void(T& front, T& back)> update_fn)
		{
			for (int i = 0; i < 2; i++)
				init_fn(buffer[i]);

			update_fn(*front, *back);

			update = update_fn;
		}

		void swap()
		{
			std::swap(front, back);
			update(*front, *back);
		}
	};
};
