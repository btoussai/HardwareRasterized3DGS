#include "GLIntrospection.h"

#include "../imgui/imgui.h"
#include <mutex>
#include <vector>
#include <cmath>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <algorithm>

std::string GLIntrospection::prettyPrintMem(size_t bytes) {
	std::stringstream ss;
	ss <<std::fixed <<std::setprecision(2);

	int n = (int)log10(bytes) / 3;

	if(n > 0 && n <= 3){
		ss <<bytes * pow(10, -n * 3);
	}else{
		ss <<bytes;
	}

	switch(n){
	case 1:
		ss <<" kB";
		break;
	case 2:
		ss <<" MB";
		break;
	case 3:
		ss <<" GB";
		break;
	default:
		ss <<" bytes";
	}

	return ss.str();
}

int GLIntrospection::getBytesPerTexel(GLenum internalFormat) {

	switch (internalFormat) {
	case GL_R8:
	case GL_R8_SNORM:
	case GL_R8I:
	case GL_R8UI:
		return 1;
	case GL_RG8:
	case GL_RG8_SNORM:
	case GL_RG8I:
	case GL_RG8UI:

	case GL_R16:
	case GL_R16_SNORM:
	case GL_R16I:
	case GL_R16UI:
	case GL_R16F:
		return 2;
	case GL_RGBA8:
	case GL_RGBA8_SNORM:
	case GL_RGBA8I:
	case GL_RGBA8UI:

	case GL_RG16:
	case GL_RG16_SNORM:
	case GL_RG16I:
	case GL_RG16UI:
	case GL_RG16F:

	case GL_R32I:
	case GL_R32UI:
	case GL_R32F:
		return 4;
	case GL_RGBA16:
	case GL_RGBA16_SNORM:
	case GL_RGBA16I:
	case GL_RGBA16UI:
	case GL_RGBA16F:

	case GL_RG32I:
	case GL_RG32UI:
	case GL_RG32F:
		return 8;
	case GL_RGBA32I:
	case GL_RGBA32UI:
	case GL_RGBA32F:
		return 16;
	}

	throw std::string("Unkown internal format: ") + std::to_string(internalFormat);
}

std::string GLIntrospection::getFormatName(GLenum internalFormat){
	switch (internalFormat) {
	case GL_R8: return "GL_R8";
	case GL_R8_SNORM: return "GL_R8_SNORM";
	case GL_R8I: return "GL_R8I";
	case GL_R8UI: return "GL_R8UI";

	case GL_RG8: return "GL_RG8";
	case GL_RG8_SNORM: return "GL_RG8_SNORM";
	case GL_RG8I: return "GL_RG8I";
	case GL_RG8UI: return "GL_RG8UI";

	case GL_R16: return "GL_R16";
	case GL_R16_SNORM: return "GL_R16_SNORM";
	case GL_R16I: return "GL_R16I";
	case GL_R16UI: return "GL_R16UI";
	case GL_R16F: return "GL_R16F";

	case GL_RGBA8: return "GL_RGBA8";
	case GL_RGBA8_SNORM: return "GL_RGBA8_SNORM";
	case GL_RGBA8I: return "GL_RGBA8I";
	case GL_RGBA8UI: return "GL_RGBA8UI";

	case GL_RG16: return "GL_RG16";
	case GL_RG16_SNORM: return "GL_RG16_SNORM";
	case GL_RG16I: return "GL_RG16I";
	case GL_RG16UI: return "GL_RG16UI";
	case GL_RG16F: return "GL_RG16F";

	case GL_R32I: return "GL_R32I";
	case GL_R32UI: return "GL_R32UI";
	case GL_R32F: return "GL_R32F";

	case GL_RGBA16: return "GL_RGBA16";
	case GL_RGBA16_SNORM: return "GL_RGBA16_SNORM";
	case GL_RGBA16I: return "GL_RGBA16I";
	case GL_RGBA16UI: return "GL_RGBA16UI";
	case GL_RGBA16F: return "GL_RGBA16F";

	case GL_RG32I: return "GL_RG32I";
	case GL_RG32UI: return "GL_RG32UI";
	case GL_RG32F: return "GL_RG32F";

	case GL_RGBA32I: return "GL_RGBA32I";
	case GL_RGBA32UI: return "GL_RGBA32UI";
	case GL_RGBA32F: return "GL_RGBA32F";
	}
	throw std::string("Unkown internal format: ") + std::to_string(internalFormat);
}

static std::mutex mut;

std::unordered_set<GLuint> GLIntrospection::textures;
std::unordered_set<GLuint> GLIntrospection::buffers;
std::unordered_set<GLuint> GLIntrospection::vaos;

void GLIntrospection::addTexture(GLuint ID) {
	if(ID == 0){
		throw std::string("Error, texture ID==0.");
	}
	std::lock_guard<std::mutex> lock(mut);
	if(textures.contains(ID)){
		throw std::string("Error, texture ID already in use: ") + std::to_string(ID);
	}
	textures.insert(ID);
}

void GLIntrospection::removeTexture(GLuint ID) {
	if(ID != 0){
		std::lock_guard<std::mutex> lock(mut);
		if(!textures.contains(ID)){
			throw std::string("Error, texture ID already removed: ") + std::to_string(ID);
		}
		textures.erase(textures.find(ID));
	}
}

void GLIntrospection::addBuffer(GLuint ID) {
	if(ID == 0){
		throw std::string("Error, buffer ID==0.");
	}
	std::lock_guard<std::mutex> lock(mut);
	if(buffers.contains(ID)){
		throw std::string("Error, buffer ID already in use: ") + std::to_string(ID);
	}
	buffers.insert(ID);
}

void GLIntrospection::removeBuffer(GLuint ID) {
	if(ID != 0){
		std::lock_guard<std::mutex> lock(mut);
		if(!buffers.contains(ID)){
			throw std::string("Error, buffer ID already removed: ") + std::to_string(ID);
		}
		buffers.erase(buffers.find(ID));
	}
}

void GLIntrospection::addVAO(GLuint ID) {
    if(ID == 0){
        throw std::string("Error, VAO ID==0.");
    }
    std::lock_guard<std::mutex> lock(mut);
    if(vaos.contains(ID)){
        throw std::string("Error, VAO ID already in use: ") + std::to_string(ID);
    }
    vaos.insert(ID);
}

void GLIntrospection::removeVAO(GLuint ID) {
    if(ID != 0){
        std::lock_guard<std::mutex> luck(mut);
        if(!vaos.contains(ID)){
            throw std::string("Error, VAO ID already removed: ") + std::to_string(ID);
        }
        vaos.erase(vaos.find(ID));
    }
}

void GLIntrospection::inspectObjects() {
	std::lock_guard<std::mutex> lock(mut);

	struct TexItem{
		GLuint ID;
		GLenum internalFormat;
		int width;
		int height;
		int depth;
		size_t size;
	};

	struct TexItemGrouped{
		std::vector<GLuint> IDs;
		GLenum internalFormat;
		int width;
		int height;
		int depth;
		size_t size;
	};


	struct BufItem{
		GLuint ID;
		size_t size;
	};

	if(ImGui::BeginMenu("OpenGL Allocations")){

		struct TexItemKey{
			GLenum internalFormat;
			int width;
			int height;
			int depth;

			bool operator==(const TexItemKey& o) const{
				return internalFormat == o.internalFormat && width == o.width && height == o.height && depth == o.depth;
			}
		};
		struct TexItemKeyHash{
			std::size_t operator()(const TexItemKey& o) const {
				std::size_t h = o.internalFormat;
				h = h * 31 + o.width;
				h = h * 31 + o.height;
				h = h * 31 + o.depth;
				return h;
			}
		};

		std::unordered_map<TexItemKey, TexItemGrouped, TexItemKeyHash> texturesGrouped;

		std::vector<BufItem> sortedBuffers;
		sortedBuffers.reserve(buffers.size());

		size_t totalTexSize = 0;
		size_t totalBufSize = 0;

		for(GLuint ID : textures){
			TexItem item;
			item.ID = ID;
			glGetTextureLevelParameteriv(item.ID, 0, GL_TEXTURE_INTERNAL_FORMAT, (int*)&item.internalFormat);
			glGetTextureLevelParameteriv(item.ID, 0, GL_TEXTURE_WIDTH, &item.width);
			glGetTextureLevelParameteriv(item.ID, 0, GL_TEXTURE_HEIGHT, &item.height);
			glGetTextureLevelParameteriv(item.ID, 0, GL_TEXTURE_DEPTH, &item.depth);
			item.size = size_t(getBytesPerTexel(item.internalFormat)) * size_t(item.width) * size_t(item.height) * size_t(item.depth);
			totalTexSize += item.size;

			TexItemKey key = TexItemKey{item.internalFormat, item.width, item.height, item.depth};

			if(texturesGrouped.contains(key)){
				TexItemGrouped& prevGroup = texturesGrouped[key];
				prevGroup.IDs.push_back(item.ID);
			}else{
				TexItemGrouped newGroup = TexItemGrouped{std::vector<GLuint>({item.ID}), item.internalFormat, item.width, item.height, item.depth, item.size};
				texturesGrouped[key] = newGroup;
			}
		}
		for(GLuint ID : buffers){
			BufItem item;
			item.ID = ID;
			glGetNamedBufferParameteri64v(item.ID, GL_BUFFER_SIZE, (int64_t*)&item.size);
			totalBufSize += item.size;
			sortedBuffers.push_back(item);
		}

		ImGui::Text("Total size: %s", prettyPrintMem(totalTexSize + totalBufSize).c_str());
		if(ImGui::TreeNode("Textures", "%d Textures: %s", (int)textures.size(), prettyPrintMem(totalTexSize).c_str())){

			std::vector<TexItemGrouped> sortedTexturesGrouped;
			sortedTexturesGrouped.reserve(texturesGrouped.size());

			for(const auto& kv : texturesGrouped){
				sortedTexturesGrouped.push_back(kv.second);
			}

			std::sort(sortedTexturesGrouped.begin(), sortedTexturesGrouped.end(), [](const auto& a, const auto& b){ return a.size * a.IDs.size() > b.size * b.IDs.size(); });

			for(int i=0; i<(int)texturesGrouped.size(); i++){
				const TexItemGrouped& item = sortedTexturesGrouped[i];
				if(item.IDs.size() == 1){
					ImGui::Text("ID %d, %s, %dx%dx%d, %s", item.IDs[0], getFormatName(item.internalFormat).c_str(), item.width, item.height, item.depth, prettyPrintMem(item.size).c_str());
				}else{
					int c = (int)item.IDs.size();
					ImGui::PushID(i);
					if(ImGui::TreeNode("Group", "%dx, %s, %dx%dx%d, Total %s (%s)", c, getFormatName(item.internalFormat).c_str(), item.width, item.height, item.depth, prettyPrintMem(item.size*c).c_str(), prettyPrintMem(item.size).c_str())){
						for(GLuint ID : item.IDs){
							ImGui::Text("ID %d", ID);
						}
						ImGui::TreePop();
					}
					ImGui::PopID();

				}

			}

			ImGui::TreePop();
		}

		if(ImGui::TreeNode("Buffers", "%d Buffers: %s", (int)buffers.size(), prettyPrintMem(totalBufSize).c_str())){

			std::sort(sortedBuffers.begin(), sortedBuffers.end(), [](const BufItem& a, const BufItem& b){ return a.size > b.size; });
			for(const BufItem& item : sortedBuffers){
				ImGui::Text("ID %d, %s", item.ID, prettyPrintMem(item.size).c_str());
			}
			ImGui::TreePop();
		}

        ImGui::Text("%d VAO(s)", (int)vaos.size());

		ImGui::EndMenu();
	}


}
