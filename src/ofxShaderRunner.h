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
		
		if (ofFile::doesFileExist(glsl_path) == false)
			return;
		
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
				code += "#extension GL_EXT_gpu_shader4 : require\n"; // for gl_VertexID
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
			begin();
			setUniform1f("Time", ofGetElapsedTimef());
			setUniform1f("Alpha", alpha);
			glDrawArrays(mode, 0, count);
			end();
		}
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
		
		codes[tag] = code;
	}
	
	void _update(ofEventArgs &e)
	{
		auto D = ofGetElapsedTimeMillis() - last_check_time;
		if (D > 100)
		{
			last_check_time = ofGetElapsedTimeMillis();
			
			auto T = std::filesystem::last_write_time(ofToDataPath(glsl_path));
			if (last_modified_time != T)
			{
				load(glsl_path);
			}
		}
	}
};
