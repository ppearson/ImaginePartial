/*
 Imagine
 Copyright 2013 Peter Pearson.

 Licensed under the Apache License, Version 2.0 (the "License");
 You may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ---------
*/

#include "geo_helper_obj.h"

#include <fstream>
#include <algorithm>

#include "geometry/geometry_instance.h"
#include "geometry/standard_geometry_instance.h"
#include "geometry/standard_geometry_operations.h"

#include "utils/string_helpers.h"
#include "utils/file_helpers.h"
#include "utils/maths/maths.h"

#include "materials/standard_material.h"

namespace Imagine
{

GeoHelperObj::GeoHelperObj()
{
}

bool GeoHelperObj::readMaterialFile(const std::string& mtlPath, bool importTextures, const std::string& customTextureSearchPath, GeoMaterials& materials)
{
	char buf[256];
	memset(buf, 0, 256);

	std::fstream fileStream(mtlPath.c_str(), std::ios::in);
	
	std::string basePath = FileHelpers::getFileDirectory(mtlPath);

	std::string line;
	line.resize(256);

	StandardMaterial* pNewMaterial = NULL;

	float r;
	float g;
	float b;
	
	bool haveDiffuseTexture = false;

	while (fileStream.getline(buf, 256))
	{
		if (buf[0] == 0 || buf[0] == '#')
			continue;

		unsigned int startIndex = 0;
		// account of some .mtl files having tabs/spaces at the beginning for material values
		while (buf[startIndex] == '\t' || buf[startIndex] == ' ')
			startIndex ++;

		if (stringCompare(buf, "newmtl", 6, startIndex))
		{
			// if it's valid, add the old one to the list
			if (pNewMaterial && !pNewMaterial->getName().empty())
			{
				materials.materials[pNewMaterial->getName()] = pNewMaterial;
			}
			// create a new one
			pNewMaterial = new StandardMaterial();
			
			haveDiffuseTexture = false;

			line.assign(buf);
			line = line.substr(startIndex);

			// check for trailing \r with line endings
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			// set the name
			std::string newName;
			newName.assign(line.substr(7));
			pNewMaterial->setName(newName);
		}
		else if (importTextures && stringCompare(buf, "map_Kd", 6, startIndex))
		{
			line.assign(buf);
			line = line.substr(startIndex);

			// check for trailing \r with line endings
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			std::string diffuseTextureMapFile;
			diffuseTextureMapFile.assign(line.substr(7));
			
			std::string finalDiffuseTextureFilename;
			if (extractPathFilenameFromTexturePathString(diffuseTextureMapFile, finalDiffuseTextureFilename))
			{
				// on the assumption that hard-coded Windows paths to the location the .obj files were created are useless,
				// just get hold of the relative filename...
				finalDiffuseTextureFilename = FileHelpers::getFileNameAllPlatforms(finalDiffuseTextureFilename);
				
				// try with relative path to location of .mtl file first
				std::string diffuseTextureMapFullPath = FileHelpers::combinePaths(basePath, finalDiffuseTextureFilename);
				if (!customTextureSearchPath.empty() && !FileHelpers::doesFileExist(diffuseTextureMapFullPath))
				{
					// now try to a custom searchpath
					diffuseTextureMapFullPath = FileHelpers::combinePaths(customTextureSearchPath, finalDiffuseTextureFilename);
				}
				pNewMaterial->setDiffuseTextureMapPath(diffuseTextureMapFullPath, false);
				
				haveDiffuseTexture = true; // make a note of the fact we extracted a diffuse texture.
			}
		}
		else if (importTextures && (stringCompare(buf, "bump", 4, startIndex) || stringCompare(buf, "map_bump", 7, startIndex)))
		{
			line.assign(buf);
			line = line.substr(startIndex);

			// check for trailing \r with line endings
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			size_t spacePos = line.find(" ");

			std::string bumpTextureMapFile;
			bumpTextureMapFile.assign(line.substr(spacePos + 1));

			std::string finalBumpTextureFilename;
			if (extractPathFilenameFromTexturePathString(bumpTextureMapFile, finalBumpTextureFilename))
			{
				// on the assumption that hard-coded Windows paths to the location the .obj files were created are useless,
				// just get hold of the relative filename...
				finalBumpTextureFilename = FileHelpers::getFileNameAllPlatforms(finalBumpTextureFilename);
				
				std::string bumpTextureMapFullPath = FileHelpers::combinePaths(basePath, finalBumpTextureFilename);
				if (!customTextureSearchPath.empty() && !FileHelpers::doesFileExist(bumpTextureMapFullPath))
				{
					// now try to a custom searchpath
					bumpTextureMapFullPath = FileHelpers::combinePaths(customTextureSearchPath, finalBumpTextureFilename);
				}
				pNewMaterial->setBumpTextureMapPath(bumpTextureMapFullPath, false);
			}
		}
		else if (importTextures && stringCompare(buf, "map_d", 5, startIndex)) // alpha texture
		{
			line.assign(buf);
			line = line.substr(startIndex);

			// check for trailing \r with line endings
			if (line[line.size() - 1] == '\r')
				line = line.substr(0, line.size() - 1);

			size_t spacePos = line.find(" ");

			std::string alphaTextureMapFile;
			alphaTextureMapFile.assign(line.substr(spacePos + 1));

			std::string finalAlphaTextureFilename;
			if (extractPathFilenameFromTexturePathString(alphaTextureMapFile, finalAlphaTextureFilename))
			{
				// on the assumption that hard-coded Windows paths to the location the .obj files were created are useless,
				// just get hold of the relative filename...
				finalAlphaTextureFilename = FileHelpers::getFileNameAllPlatforms(finalAlphaTextureFilename);
				
				std::string alphaTextureMapFullPath = FileHelpers::combinePaths(basePath, finalAlphaTextureFilename);
				if (!customTextureSearchPath.empty() && !FileHelpers::doesFileExist(alphaTextureMapFullPath))
				{
					// now try to a custom searchpath
					alphaTextureMapFullPath = FileHelpers::combinePaths(customTextureSearchPath, finalAlphaTextureFilename);
				}
				pNewMaterial->setAlphaTextureMapPath(alphaTextureMapFullPath, false);
			}
		}
		else if (buf[startIndex] == 'K' && buf[startIndex + 1] == 'd')
		{
			// .mtl files often have diffuse texture maps AND Kd constant colours
			// in arbitrary orders. In this case, if we've seen a diffuse texture already,
			// skip setting the diffuse constant colour to prevent overwriting the texture...
			if (!haveDiffuseTexture)
			{
				sscanf(buf + startIndex, "Kd %f %f %f", &r, &g, &b);
				pNewMaterial->setDiffuseColour(Colour3f(r, g, b));
			}
		}
		else if (buf[startIndex] == 'K' && buf[startIndex + 1] == 's')
		{
			sscanf(buf + startIndex, "Ks %f %f %f", &r, &g, &b);
			pNewMaterial->setSpecularColour(Colour3f(r, g, b));
		}
		else if (buf[startIndex] == 'N' && buf[startIndex + 1] == 's')
		{
			float shininess;
			sscanf(buf + startIndex, "Ns %f", &shininess);
			
			// Obj .mtl specs say higher is tighter (i.e. less rough, more specular), but unfortunately,
			// it seems that's often not the case, so there's not really a lot we can do
			// that's robust in all situations...
			// A large number of .mtl files I've seen that do obey the higher is tighter spec,
			// seem to assume ~100-130 is fully specular (coming from OpenGL 2.x limits?),
			// so work on that (broken) assumption for the moment.

			// convert back to our range
			float roughnessValue = fit(shininess, 0.0f, 128.0f, 0.0f, 1.0f);
			pNewMaterial->setSpecularRoughness(1.0f - roughnessValue);
		}
		else if (buf[startIndex] == 'd' && buf[startIndex + 1] == ' ')
		{
			float transparency;
			sscanf(buf + startIndex, "d %f", &transparency);
			pNewMaterial->setTransparancy(1.0f - transparency);
		}
/*		else if (buf[startIndex] == 'T' && buf[startIndex + 1] == 'r')
		{
			float transparency;
			sscanf(buf + startIndex, "Tr %f", &transparency);
			pNewMaterial->setTransparancy(1.0f - transparency);
		}
*/
	}

	fileStream.close();

	// make sure the last material gets added
	if (pNewMaterial && !pNewMaterial->getName().empty())
	{
		materials.materials[pNewMaterial->getName()] = pNewMaterial;
	}

	if (materials.materials.empty())
		return false;

	return true;
}

// Attempt to work out which bit is the filename, and discard the rest (probably options).
// If we didn't care about spaces in the filename, this would be easy (on the assumption that
// all paths will have a file extension), but alas most Obj files (that I've seen anyway)
// seem to be authored in Windows and they often have spaces in texture map filenames...
//
// This is unfortunately quite tricky to always get right - the Wavefront specs says options for filenames should be *before*
// the filename, but this often isn't the case (they're after the filename), so we need to try and work it out by
// detecting where the file extension is...
// It's somewhat questionable what the point of doing this is (given it's fairly complicated, and .mtl files often have
// hardcoded Windows paths in them which won't exist anyway), but it does generally allow stripping options off certain
// relative map paths in .mtl files I have correctly, which wouldn't work without this logic (given spaces in filenames)
//
// Example strings we need to cope with:
//
// 1. bump bump_texture_map.jpg        - easy
// 2. bump bump texture map.jpg        - spaces in filename
// 3. bump -bm 2 bump_texture_map.jpg  - options correctly before filename
// 4. bump -bm 2 bump texture map.jpg  - options correctly before filename with spaces in filename
// 5. bump bump_texture_map.jpg -bm 2  - options incorrectly after filename
// 6. bump bump texture map.jpg -bm 2  - options incorrectly after filename with spaces in filename
// 7. bump bump -texture-map.jpg -bm 2 - filename with space and hyphen is before options - 
//                                       we're screwed if this happens - we'd need to have a white list of options
//                                       to recognise them correctly compared to the filename...
// Note: currently, 1-6 are handled, but given 7 is extremely unlikely and more complex to handle, this isn't currently handled.
bool GeoHelperObj::extractPathFilenameFromTexturePathString(const std::string& originalPathString, std::string& finalFilename)
{
	if (originalPathString.empty())
		return false;
	
	// assume the filename will have a "." char in it - technically (UNIX) it doesn't need to, but in practice, it always will,
	// so we can use this fact to detect which bit is the filename
	if (originalPathString.find(".") == std::string::npos)
		return false;
	
	std::vector<std::string> tokens;
	splitString(originalPathString, tokens, " ", 0);
	
	if (tokens.size() == 1)
	{
		// we've only got one string item, so it must be that
		finalFilename = originalPathString;
		return true;
	}
	
	// now go through the tokens, and attempt to work out if there are any options and if so, where they are in relation to the filename.
	
	// To do this completely robustly, we should be able to understand each possible option, and interpret it to some degree (as options can
	// have different numbers of arguments), however for our purposes it's generally good enough to just work out which one is first based
	// off where a filename extension is and where options are - the assumptions being options will be prefixed with "-" and will all be together
	// either before OR after the filename - if these assumptions aren't met, then we can't cope, and we don't bother parsing the filename
	
	size_t extensionPos = -1u;
	size_t optionsFirstPos = -1u; // index of first option item
	size_t optionsLastPos = -1u;
	
	size_t index = 0;
	
	// TODO: we could attempt to do this in one pass, but...
	
	std::vector<std::string>::iterator itToken = tokens.begin();
	for (; itToken != tokens.end(); ++itToken)
	{
		const std::string& tokenString = *itToken;
		
		bool isOption = (tokenString.substr(0, 1) == "-");
		bool containsExtension = (tokenString.find(".") != std::string::npos);
		// check it's really an extension with a non-numeric character after the last ".".
		// This is unfortunately necessary due to options like -bm 0.18 which can come after a real filename.
		// To cope with this properly we'd need to detect options and their args in this loop and skip them
		// (which is looking like the best approach in hindsight).
		if (containsExtension)
		{
			size_t finalDot = tokenString.find_last_of('.');
			if (finalDot != std::string::npos && (tokenString.size() > finalDot + 1))
			{
				containsExtension &= !isdigit(tokenString[finalDot + 1]);
			}
			else
			{
				// otherwise, assume it isn't a valid file extension
				containsExtension = false;
			}
		}

		if (isOption && !containsExtension)
		{
			// just an option
			if (optionsFirstPos == -1u)
			{
				optionsFirstPos = index;
			}
			optionsLastPos = index;
		}
		else if (!isOption && containsExtension)
		{
			// just contains an extension
			extensionPos = index;
		}
		else if (isOption && containsExtension)
		{
			// both. Oh dear...
			// On the assumption that there isn't an actual option token (i.e. the option command as opposed to option argument) with
			// "." in it, let's assume it's the extension part
			extensionPos = index;
		}
		else
		{
			// otherwise, we assume it's either a part of a filename that has spaces in, or is an argument for an option
		}
		
		index ++;
	}
	
	// this shouldn't happen, but...
	if (extensionPos == -1u)
	{
		return false;
	}
	
	// if we don't have any options, assume passed in string was entire filename
	if (optionsFirstPos == -1u && optionsLastPos == -1u)
	{
		finalFilename = originalPathString;
		return true;
	}
	
	// if the options were after the extension pos, it's easy - we can assume all tokens up to and including the extension pos
	// token are the filename
	if (optionsFirstPos > extensionPos && optionsLastPos > extensionPos)
	{
		finalFilename = tokens[0];
		
		for (unsigned int i = 1; i <= extensionPos; i++)
		{
			finalFilename += " ";
			finalFilename += tokens[i];
		}
		
		return true;
	}
	else if (optionsFirstPos < extensionPos && optionsLastPos < extensionPos)
	{
		// hackily progress through the options (and attempt to correctly skip the arguments)
		index = 0;
		
		itToken = tokens.begin();
		for (; itToken != tokens.end(); ++itToken)
		{
			const std::string& tokenString = *itToken;
			
			bool isOption = (tokenString.substr(0, 1) == "-");
			
			// as we skip args explicitly (below), when we get to this stage,
			// we know we're at the end of the args and this is the first filename
			// item
			if (!isOption)
				break;

			std::string optionName = tokenString.substr(1);
			
			unsigned int argCount = 1;
			
			if (optionName == "o" || optionName == "s" || optionName == "t")
			{
				// -o -s -t have: u, v, [w] args
				argCount = 2;
				// the only way to know if there is an optional third w argument is to see if the next
				// arg is a number or not, which is obviously not completely robust - i.e. we could have
				// the following:
				// map_Kd -s 1 0.25 1 my lovely texture.jpg
				// where the texture name is "1 my lovely texture.jpg"
				// in which case, checking if the third string token is a number doesn't tell us anything useful.
				// The only way to really deal with this is try and stat all filename combinations we see in order
				// to work out what the actual filename is, which is pretty silly really, and would complicate
				// this function even more to do.
				// As such, for the moment, just assume these options only have two arguments (which is all I've ever
				// seen in .mtl files I've got), in a less-than-ideal compromise. The alternative hacky solution
				// is not coping with filenames with spaces starting with numbers, but unfortunately these exist...
			}
			else if (optionName == "mm")
			{
				argCount = 2;
			}
			
			index += 1;
			
			// skip over the args
			for (unsigned int j = 0; j < argCount; j++)
			{
				++itToken; 
			}
			
			index += argCount;
		}
		
		// now append further tokens to final string
		finalFilename = tokens[index];
		
		for (unsigned int i = index + 1; i < tokens.size(); i++)
		{
			finalFilename += " ";
			finalFilename += tokens[i];
		}
		
		return true;
	}
	
	return false;
}

void GeoHelperObj::copyPointsToGeometry(std::vector<Point>& points, std::set<unsigned int>& pointIndexesRequired, EditableGeometryInstance* pGeoInstance)
{
	std::deque<Point>& geoPoints = pGeoInstance->getPoints();
	std::map<unsigned int, unsigned int> aMapToFaceVertices;

	unsigned int localIndex = 0;

	std::set<unsigned int>::iterator it = pointIndexesRequired.begin();
	for (; it != pointIndexesRequired.end(); ++it)
	{
		unsigned int index = *it;
		geoPoints.push_back(points[index]);

		aMapToFaceVertices[index] = localIndex;

		localIndex++;
	}

	// fix up faces so they only
	std::deque<Face>& geoFaces = pGeoInstance->getFaces();
	
	std::vector<unsigned int> aNewFaceIndexes;

	std::deque<Face>::iterator itFace = geoFaces.begin();
	for (; itFace != geoFaces.end(); ++itFace)
	{
		Face& face = *itFace;

		aNewFaceIndexes.clear();

		unsigned int vertexCount = face.getVertexCount();
		for (unsigned int i = 0; i < vertexCount; i++)
		{
			unsigned int vertexIndex = face.getVertexPosition(i);

//			std::map<unsigned int, unsigned int>::iterator it1 = aMapToFaceVertices.find(vertexIndex);

			unsigned int newIndex = aMapToFaceVertices[vertexIndex];

			aNewFaceIndexes.push_back(newIndex);
		}

		face.clear();

		for (unsigned int i = 0; i < vertexCount; i++)
		{
			unsigned int vertexIndex = aNewFaceIndexes[i];
			face.addVertex(vertexIndex);
		}

		face.calculateNormal(pGeoInstance);
	}
}

void GeoHelperObj::copyUVsToGeometry(std::vector<UV>& uvs, std::set<unsigned int>& vertexUVsRequired, EditableGeometryInstance* pGeoInstance)
{
	if (vertexUVsRequired.empty())
		return;

	std::deque<UV>& geoUVs = pGeoInstance->getUVs();
	std::map<unsigned int, unsigned int> aMapToFaceUVs;

	unsigned int localIndex = 0;

	std::set<unsigned int>::iterator it = vertexUVsRequired.begin();
	for (; it != vertexUVsRequired.end(); ++it)
	{
		unsigned int index = *it;
		geoUVs.push_back(uvs[index]);

		aMapToFaceUVs[index] = localIndex;

		localIndex++;
	}

	if (!geoUVs.empty())
		pGeoInstance->setHasPerVertexUVs(true);

	// fix up faces so they only
	std::deque<Face>& geoFaces = pGeoInstance->getFaces();

	std::vector<unsigned int> aNewFaceIndexes;
	
	std::deque<Face>::iterator itFace = geoFaces.begin();
	for (; itFace != geoFaces.end(); ++itFace)
	{
		Face& face = *itFace;

		aNewFaceIndexes.clear();

		unsigned int vertexCount = face.getVertexCount();
		for (unsigned int i = 0; i < vertexCount; i++)
		{
			unsigned int UVIndex = face.getVertexUV(i);

//			std::map<unsigned int, unsigned int>::iterator it1 = aMapToFaceUVs.find(UVIndex);
//			assert(it1 != aMapToFaceUVs.end());

			unsigned int newIndex = aMapToFaceUVs[UVIndex];

			aNewFaceIndexes.push_back(newIndex);
		}

		face.clearUVs();

		for (unsigned int i = 0; i < vertexCount; i++)
		{
			unsigned int vertexIndex = aNewFaceIndexes[i];
			face.addUV(vertexIndex);
		}
	}
}

void GeoHelperObj::calculateFaceNormalsForGeometry(EditableGeometryInstance* pGeoInstance)
{
	std::deque<Face>& geoFaces = pGeoInstance->getFaces();

	std::deque<Face>::iterator it = geoFaces.begin();
	for (; it != geoFaces.end(); ++it)
	{
		Face& face = *it;
		face.calculateNormal(pGeoInstance);
	}
}

void GeoHelperObj::copyPointItemsToGeometry(std::vector<Point>& points, std::set<unsigned int>& pointIndexesRequired,
									StandardGeometryInstance* pGeoInstance)
{
	// points

	std::vector<Point>& geoPoints = pGeoInstance->getPoints();
	std::map<unsigned int, unsigned int> aMapToFaceVertices;

	unsigned int localIndex = 0;

	std::set<unsigned int>::iterator it = pointIndexesRequired.begin();
	for (; it != pointIndexesRequired.end(); ++it)
	{
		unsigned int index = *it;
		geoPoints.push_back(points[index]);

		aMapToFaceVertices[index] = localIndex;

		localIndex++;
	}

	std::vector<uint32_t>::iterator itPolyIndices = pGeoInstance->getPolygonIndices().begin();
	for (; itPolyIndices != pGeoInstance->getPolygonIndices().end(); ++itPolyIndices)
	{
		uint32_t& vertexIndex = *itPolyIndices;

		unsigned int newIndex = aMapToFaceVertices[vertexIndex];

		vertexIndex = newIndex;
	}
}

void GeoHelperObj::copyUVItemsToGeometry(std::vector<UV>& uvs, std::set<unsigned int>& vertexUVsRequired, StandardGeometryInstance* pGeoInstance)
{
	if (vertexUVsRequired.empty())
		return;
	
	std::vector<UV>& geoUVs = pGeoInstance->getUVs();
	std::map<unsigned int, unsigned int> aMapToFaceVerticesUV;

	unsigned int localIndex = 0;

	std::set<unsigned int>::iterator it = vertexUVsRequired.begin();
	for (; it != vertexUVsRequired.end(); ++it)
	{
		unsigned int index = *it;
		geoUVs.push_back(uvs[index]);

		aMapToFaceVerticesUV[index] = localIndex;

		localIndex++;
	}

	unsigned int numUVIndices = pGeoInstance->getUVIndicesCount();
	uint32_t* pUVI = const_cast<uint32_t*>(pGeoInstance->m_pUVIndices);
	for (unsigned int i = 0; i < numUVIndices; i++)
	{
		uint32_t& vertexIndex = *pUVI;

		unsigned int newIndex = aMapToFaceVerticesUV[vertexIndex];

		vertexIndex = newIndex;

		pUVI++;
	}
}

} // namespace Imagine
