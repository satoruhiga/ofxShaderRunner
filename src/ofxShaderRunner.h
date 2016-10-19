#pragma once

#include "ofMain.h"

#include <Poco/Process.h>
#include <Poco/Pipe.h>
#include <Poco/PipeStream.h>
#include <Poco/StreamCopier.h>
#include <Poco/Environment.h>

class ofxShaderRunner : public ofShader
{
public:
	
	struct Arguments {
		string vertex;
		string fragment;
		string geometry;
		string compute;
		
		Arguments()
			: vertex("vertex")
			, fragment("fragment")
			, geometry("geometry")
			, compute("compute")
		{}
	};
	
	~ofxShaderRunner()
	{
		disableAutoReload();
	}
	
	bool load(const string& glsl_path,
			  const Arguments& args = Arguments())
	{
		unload();
		this->glsl_path = glsl_path;

		// set dummy vertex
		ofVec3f v(0);
		vbo.setVertexData(&v, 1, GL_STATIC_DRAW);

		if (ofFile::doesFileExist(glsl_path) == false)
			return false;
		
		this->args = args;
		
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

				if (tag == args.vertex)
				{
					if (ofGLCheckExtension("GL_EXT_gpu_shader4"))
						code += "#extension GL_EXT_gpu_shader4 : require\n"; // for gl_VertexID
				}
				else if (tag == args.geometry)
				{
					if (ofGLCheckExtension("GL_EXT_geometry_shader4"))
						code += "#extension GL_EXT_geometry_shader4 : enable\n"; // for geometry shader
				}
				else if (tag == args.compute)
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
	
	bool load(const string& glsl_path,
			  const map<string, string>& map_args)
	{
		Arguments args;
		
		if (map_args.find("vertex") != map_args.end())
			args.vertex = map_args.at("vertex");
		
		if (map_args.find("fragment") != map_args.end())
			args.fragment = map_args.at("fragment");

		if (map_args.find("geometry") != map_args.end())
			args.geometry = map_args.at("geometry");

		if (map_args.find("compute") != map_args.end())
			args.compute = map_args.at("compute");
		
		return load(glsl_path, args);
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
		if (use_point_size)
		{
			state.s_GL_PROGRAM_POINT_SIZE = glIsEnabled(GL_PROGRAM_POINT_SIZE);
			glEnable(GL_PROGRAM_POINT_SIZE);
		}
		
		ofShader::begin();
		setUniform1f("Time", ofGetElapsedTimef());
		setUniform1f("TimeInc", ofGetLastFrameTime());
		setUniform1f("Alpha", alpha);
	}

	void end()
	{
		ofShader::end();
		
		if (use_point_size && state.s_GL_PROGRAM_POINT_SIZE == false)
			glDisable(GL_PROGRAM_POINT_SIZE);
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
	bool use_point_size = false;
	
	struct EnableState {
		bool s_GL_PROGRAM_POINT_SIZE;
	} state;

	ofVbo vbo;
	
	Arguments args;
	
	void setCode(const string& tag, string code)
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
				else if (e[0] == "use_point_size")
				{
					use_point_size = e[1] == "true" || e[1] == "1";
				}
			}
		}
		else if (tag == args.vertex)
		{
			if (glslify.isValid()) glslify.process(code, code);
			setupShaderFromSource(GL_VERTEX_SHADER, code);
		}
		else if (tag == args.fragment)
		{
			if (glslify.isValid()) glslify.process(code, code);
			setupShaderFromSource(GL_FRAGMENT_SHADER, code);
		}
		else if (tag == args.geometry)
		{
			if (glslify.isValid()) glslify.process(code, code);
			setupShaderFromSource(GL_GEOMETRY_SHADER, code);
		}
		else if (tag == args.compute)
		{
			if (glslify.isValid()) glslify.process(code, code);
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
				load(glsl_path, args);
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
	
public:
	
	struct Glslify {
		
		string node_modules_path;
		vector<string> PATH;
		
		bool valid = false;
		
		Glslify() {
			node_modules_path = ofToDataPath("node_modules", true);
			appendPath("/usr/local/bin"); // linux like os
		}
		
		string getPath() const {
			return ofFilePath::join(node_modules_path, ".bin/glslify");
		}
		
		void appendPath(const string& path) {
			PATH.emplace_back(path);
		}
		
		bool isValid() {
			if (valid) return valid;
			
			string bin_path = getPath();
			string PATH_str = Poco::Environment::get("PATH")
				+ ":" + ofJoinString(PATH, ":");

			try
			{
				Poco::Pipe outPipe;
				Poco::Process::Args args { "-v" };
				Poco::Process::Env env { {"PATH", PATH_str} };
				Poco::ProcessHandle ph = Poco::Process::launch(bin_path, args,
															   0, &outPipe, 0, env);

				valid = ph.wait() == 0;
			}
			catch (std::exception& e)
			{
				valid = false;
			}
			
			return valid;
		}
		
		bool process(const string& in_code, string& out_code) {
			string PATH_str = ofJoinString(PATH, ":");
			string bin_path = getPath();
			string pwd = ofFilePath::getEnclosingDirectory(node_modules_path);
			
			Poco::Pipe inPipe;
			Poco::Pipe outPipe;
			Poco::Process::Args args {};
			Poco::Process::Env env { {"PATH", PATH_str} };
			Poco::ProcessHandle ph = Poco::Process::launch(bin_path, args, pwd,
														   &inPipe, &outPipe, 0, env);
			
			Poco::PipeOutputStream ostr(inPipe);
			Poco::PipeInputStream istr(outPipe);
			
			ostr << in_code;
			ostr.close();
			
			stringstream ss;
			Poco::StreamCopier::copyStream(istr, ss);
			
			int rc = ph.wait();
			if (rc == 0)
			{
				out_code = ss.str();
				return true;
			}
			else
			{
				out_code = in_code;
				return false;
			}
		}
		
	} glslify;
};
