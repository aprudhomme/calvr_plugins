#include <GL/glew.h>
#include "PanoDrawableLOD.h"

#include <cvrConfig/ConfigManager.h>
#include <cvrKernel/NodeMask.h>
#include <cvrKernel/ScreenConfig.h>
#include <cvrKernel/PluginHelper.h>
#include <cvrKernel/ScreenBase.h>

#include <osg/GraphicsContext>

#include <iostream>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>

#include "sph-cache.hpp"

//#define PRINT_TIMING

#include <sys/time.h>

std::map<int,std::vector<int> > PanoDrawableLOD::_leftFileIDs;
std::map<int,std::vector<int> > PanoDrawableLOD::_rightFileIDs;
std::map<int,bool> PanoDrawableLOD::_updateDoneMap;
std::map<int,int> PanoDrawableLOD::_initMap;
OpenThreads::Mutex PanoDrawableLOD::_initLock;
std::map<int,OpenThreads::Mutex*> PanoDrawableLOD::_updateLock;
std::map<int,sph_cache*> PanoDrawableLOD::_cacheMap;
std::map<int,sph_model*> PanoDrawableLOD::_modelMap;

using namespace cvr;

struct MyGLClientState
{
    GLint abuffer;
    GLint ebuffer;
    GLboolean carray;
    GLboolean efarray;
    GLboolean farray;
    GLboolean iarray;
    GLboolean narray;
    GLboolean scarray;
    GLboolean tarray;
    GLboolean varray;
};

void pushClientState(MyGLClientState & state)
{
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&state.abuffer);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING,&state.ebuffer);
    state.carray = glIsEnabled(GL_COLOR_ARRAY);
    state.efarray = glIsEnabled(GL_EDGE_FLAG_ARRAY);
    state.farray = glIsEnabled(GL_FOG_COORD_ARRAY);
    state.iarray = glIsEnabled(GL_INDEX_ARRAY);
    state.narray = glIsEnabled(GL_NORMAL_ARRAY);
    state.scarray = glIsEnabled(GL_SECONDARY_COLOR_ARRAY);
    state.tarray = glIsEnabled(GL_TEXTURE_COORD_ARRAY);
    state.varray = glIsEnabled(GL_VERTEX_ARRAY);

    if(state.carray)
    {
	glDisableClientState(GL_COLOR_ARRAY);
    }
    if(state.efarray)
    {
	glDisableClientState(GL_EDGE_FLAG_ARRAY);
    }
    if(state.farray)
    {
	glDisableClientState(GL_FOG_COORD_ARRAY);
    }
    if(state.iarray)
    {
	glDisableClientState(GL_INDEX_ARRAY);
    }
    if(state.narray)
    {
	glDisableClientState(GL_NORMAL_ARRAY);
    }
    if(state.scarray)
    {
	glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
    }
    if(state.tarray)
    {
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    if(state.varray)
    {
	glDisableClientState(GL_VERTEX_ARRAY);
    }
}

void popClientState(MyGLClientState & state)
{
    if(state.carray)
    {
	glEnableClientState(GL_COLOR_ARRAY);
    }
    if(state.efarray)
    {
	glEnableClientState(GL_EDGE_FLAG_ARRAY);
    }
    if(state.farray)
    {
	glEnableClientState(GL_FOG_COORD_ARRAY);
    }
    if(state.iarray)
    {
	glEnableClientState(GL_INDEX_ARRAY);
    }
    if(state.narray)
    {
	glEnableClientState(GL_NORMAL_ARRAY);
    }
    if(state.scarray)
    {
	glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
    }
    if(state.tarray)
    {
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    if(state.varray)
    {
	glEnableClientState(GL_VERTEX_ARRAY);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state.ebuffer);
    glBindBuffer(GL_ARRAY_BUFFER, state.abuffer);
}

void printGLState()
{
    std::cerr << "Draw State:" << std::endl;
    std::cerr << "ColorArray: " << (int)glIsEnabled(GL_COLOR_ARRAY) << std::endl;
    std::cerr << "EdgeFlag: " << (int)glIsEnabled(GL_EDGE_FLAG_ARRAY) << std::endl;
    std::cerr << "Fog: " << (int)glIsEnabled(GL_FOG_COORD_ARRAY) << std::endl;
    std::cerr << "Index: " << (int)glIsEnabled(GL_INDEX_ARRAY) << std::endl;
    std::cerr << "Normal: " << (int)glIsEnabled(GL_NORMAL_ARRAY) << std::endl;
    std::cerr << "Sec Color: " << (int)glIsEnabled(GL_SECONDARY_COLOR_ARRAY) << std::endl;
    std::cerr << "Texture: " << (int)glIsEnabled(GL_TEXTURE_COORD_ARRAY) << std::endl;
    std::cerr << "Vertex: " << (int)glIsEnabled(GL_VERTEX_ARRAY) << std::endl;
}

void clearGLClientState()
{
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_EDGE_FLAG_ARRAY);
    glDisableClientState(GL_FOG_COORD_ARRAY);
    glDisableClientState(GL_INDEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

char * loadShaderFile(std::string file)
{
    struct stat st;
    if(stat(file.c_str(),&st) != 0)
    {
        std::cerr << "Error stating shader file: " << file << std::endl;
        return NULL;
    }

    char * fileBuffer;
    int filefd;
    filefd = open(file.c_str(),O_RDONLY);
    if(!filefd)
    {
        std::cerr << "Error opening shader file: " << file << std::endl;
        return NULL;
    }

    fileBuffer = new char[st.st_size+1];
    fileBuffer[st.st_size] = '\0';
    read(filefd,fileBuffer,st.st_size);

    close(filefd);

    return fileBuffer;
}

PanoDrawableLOD::PanoDrawableLOD(std::string leftEyeFile, std::string rightEyeFile, float radius, int mesh, int depth, int size, std::string vertFile, std::string fragFile)
{
    setUseDisplayList(false);
    _badInit = false;
    _leftEyeFiles.push_back(leftEyeFile);
    _rightEyeFiles.push_back(rightEyeFile);
    _radius = radius;
    _mesh = mesh;
    _depth = depth;
    _size = size;
    _currentIndex = 0;
    _totalFadeTime = ConfigManager::getFloat("value","Plugin.PanoViewLOD.FadeTime",2.0);
    _currentFadeTime = 0.0;

    std::string shaderDir = ConfigManager::getEntry("value","Plugin.PanoViewLOD.ShaderDir","");
    _vertData = loadShaderFile(shaderDir + "/" + vertFile);
    _fragData = loadShaderFile(shaderDir + "/" + fragFile);

    //std::cerr << "Vertfile: " << vertFile << " fragFile: " << fragFile << std::endl;

    if(!_vertData)
    {
	std::cerr << "Error loading shader file: " << shaderDir + "/" + vertFile << std::endl;
	_badInit = true;
    }
    
    if(!_fragData)
    {
	std::cerr << "Error loading shader file: " << shaderDir + "/" + fragFile << std::endl;
	_badInit = true;
    }

    setUpdateCallback(new PanoUpdate());
}

PanoDrawableLOD::PanoDrawableLOD(std::vector<std::string> & leftEyeFiles, std::vector<std::string> & rightEyeFiles, float radius, int mesh, int depth, int size, std::string vertFile, std::string fragFile)
{
    setUseDisplayList(false);
    _badInit = false;
    _leftEyeFiles = leftEyeFiles;
    _rightEyeFiles = rightEyeFiles;
    _radius = radius;
    _mesh = mesh;
    _depth = depth;
    _size = size;
    _currentIndex = 0;
    _totalFadeTime = ConfigManager::getFloat("value","Plugin.PanoViewLOD.FadeTime",2.0);
    _currentFadeTime = 0.0;

    std::string shaderDir = ConfigManager::getEntry("value","Plugin.PanoViewLOD.ShaderDir","");
    _vertData = loadShaderFile(shaderDir + "/" + vertFile);
    _fragData = loadShaderFile(shaderDir + "/" + fragFile);

    //std::cerr << "Vertfile: " << vertFile << " fragFile: " << fragFile << std::endl;

    if(!_vertData)
    {
	std::cerr << "Error loading shader file: " << shaderDir + "/" + vertFile << std::endl;
	_badInit = true;
    }
    
    if(!_fragData)
    {
	std::cerr << "Error loading shader file: " << shaderDir + "/" + fragFile << std::endl;
	_badInit = true;
    }

    if(_leftEyeFiles.size() == 0 || _rightEyeFiles.size() == 0)
    {
	std::cerr << "PanoDrawableLOD error: empty file list." << std::endl;
	_badInit = true;
    }

    if(_leftEyeFiles.size() != _rightEyeFiles.size())
    {
	std::cerr << "PanoDrawableLOD error: files list sizes do not match." << std::endl;
	_badInit = true;
    }

    setUpdateCallback(new PanoUpdate());
}

PanoDrawableLOD::PanoDrawableLOD(const PanoDrawableLOD&,const osg::CopyOp& copyop)
{
    std::cerr << "PanoDrawableLOD Warning: in copy constructor." << std::endl;
}

PanoDrawableLOD::~PanoDrawableLOD()
{
    if(_vertData)
    {
	delete[] _vertData;
    }
    if(_fragData)
    {
	delete[] _fragData;
    }
}

void PanoDrawableLOD::cleanup()
{
    _leftFileIDs.clear();
    _rightFileIDs.clear();
    _updateDoneMap.clear();
    _initMap.clear();
    //for(std::map<int,sph_cache*>::iterator it = _cacheMap.begin(); it != _cacheMap.end(); it++)
    //{
	//make current
	//if(it->first < ScreenConfig::instance()->getNumWindows())
	//{
	    //ScreenConfig::instance()->getWindowInfo(it->first)->gc->makeCurrent();
	//}
	//delete it->second;
	//delete _modelMap[it->first];
    //}
    //_cacheMap.clear();
    //_modelMap.clear();
}

void PanoDrawableLOD::next()
{
    if(_leftEyeFiles.size() < 2)
    {
	return;
    }
    _lastIndex = _currentIndex;
    _currentIndex = (_currentIndex+1) % _leftEyeFiles.size();
    _nextIndex = (_currentIndex+1) % _leftEyeFiles.size();

    _currentFadeTime = _totalFadeTime + PluginHelper::getLastFrameDuration();

    for(std::map<int,std::vector<int> >::iterator it = _leftFileIDs.begin(); it != _leftFileIDs.end(); it++)
    {
        if(it->second.size())
        {
            sph_cache::_diskCache->setLeftFiles(it->second[_lastIndex],it->second[_currentIndex],it->second[_nextIndex]);
            break;
        }
    }

    for(std::map<int,std::vector<int> >::iterator it = _rightFileIDs.begin(); it != _rightFileIDs.end(); it++)
    {
        if(it->second.size())
        {
            sph_cache::_diskCache->setRightFiles(it->second[_lastIndex],it->second[_currentIndex],it->second[_nextIndex]);
            break;
        }
    }

    /*if(_leftFileIDs.size())
    {
        
        if(_leftFileIDs.begin()->second.size())
        {
            sph_cache::_diskCache->setLeftFiles(_leftFileIDs.begin()->second[_lastIndex],_leftFileIDs.begin()->second[_currentIndex],_leftFileIDs.begin()->second[_nextIndex]);
        }

        if(_rightFileIDs.begin()->second.size())
        {
            sph_cache::_diskCache->setRightFiles(_rightFileIDs.begin()->second[_lastIndex],_rightFileIDs.begin()->second[_currentIndex],_rightFileIDs.begin()->second[_nextIndex]);
        }
	//sph_cache::_diskCache->kill_tasks(_leftFileIDs.begin()->second[_lastIndex]);
	//sph_cache::_diskCache->kill_tasks(_rightFileIDs.begin()->second[_lastIndex]);
    }*/
}

void PanoDrawableLOD::previous()
{
    if(_leftEyeFiles.size() < 2)
    {
	return;
    }
    _lastIndex = _currentIndex;
    _currentIndex = (_currentIndex+_leftEyeFiles.size()-1) % _leftEyeFiles.size();
    _nextIndex = (_currentIndex+_leftEyeFiles.size()-1) % _leftEyeFiles.size();

    _currentFadeTime = _totalFadeTime + PluginHelper::getLastFrameDuration();

    for(std::map<int,std::vector<int> >::iterator it = _leftFileIDs.begin(); it != _leftFileIDs.end(); it++)
    {
        if(it->second.size())
        {
            sph_cache::_diskCache->setLeftFiles(it->second[_lastIndex],it->second[_currentIndex],it->second[_nextIndex]);
            break;
        }
    }

    for(std::map<int,std::vector<int> >::iterator it = _rightFileIDs.begin(); it != _rightFileIDs.end(); it++)
    {
        if(it->second.size())
        {
            sph_cache::_diskCache->setRightFiles(it->second[_lastIndex],it->second[_currentIndex],it->second[_nextIndex]);
            break;
        }
    }

    /*if(_leftFileIDs.size())
    {
        if(_leftFileIDs.begin()->second.size())
        {
	    sph_cache::_diskCache->setLeftFiles(_leftFileIDs.begin()->second[_lastIndex],_leftFileIDs.begin()->second[_currentIndex],_leftFileIDs.begin()->second[_nextIndex]);
        }

        if(_rightFileIDs.begin()->second.size())
        {
	    sph_cache::_diskCache->setRightFiles(_rightFileIDs.begin()->second[_lastIndex],_rightFileIDs.begin()->second[_currentIndex],_rightFileIDs.begin()->second[_nextIndex]);
        }
	//sph_cache::_diskCache->kill_tasks(_leftFileIDs.begin()->second[_lastIndex]);
	//sph_cache::_diskCache->kill_tasks(_rightFileIDs.begin()->second[_lastIndex]);
    }*/
}

void PanoDrawableLOD::setZoom(osg::Vec3 dir, float k)
{
    for(std::map<int,sph_model*>::iterator it = _modelMap.begin(); it!= _modelMap.end(); it++)
    {
	it->second->set_zoom(dir.x(),dir.y(),dir.z(),k);
    }
}

osg::BoundingBox PanoDrawableLOD::computeBound() const
{
    osg::Vec3 size2(_radius, _radius, _radius);
    _boundingBox.init();
    _boundingBox.set(-size2[0], -size2[1], -size2[2], size2[0], size2[1], size2[2]);
    return _boundingBox;
}

void PanoDrawableLOD::updateBoundingBox()
{
    computeBound();
    dirtyBound();
}

void PanoDrawableLOD::drawImplementation(osg::RenderInfo& ri) const
{
    if(_badInit)
    {
	return;
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    int context = ri.getContextID();

    int eye = 0;
    osg::Node::NodeMask parentMask;

    if(!getNumParents())
    {
	glPopAttrib();
	return;
    }

    parentMask = getParent(0)->getNodeMask();

    if((parentMask & CULL_MASK_LEFT) || (parentMask & CULL_MASK) || (ScreenConfig::instance()->getEyeSeparationMultiplier() == 0.0))
    {
	if(ScreenBase::getEyeSeparation() >= 0)
	{
	    eye = DRAW_LEFT;
	}
	else
	{
	    eye = DRAW_RIGHT;
	}
    }
    else
    {
	if(ScreenBase::getEyeSeparation() >= 0)
	{
	    eye = DRAW_RIGHT;
	}
	else
	{
	    eye = DRAW_LEFT;
	}
    }

    _initLock.lock();

    if(!_initMap[context])
    {
	if(!_cacheMap[context])
	{
	    GLenum err = glewInit();
	    if (GLEW_OK != err)
	    {
		std::cerr << "Error on glew init: " << glewGetErrorString(err) << std::endl;
		_badInit = true;
		_initLock.unlock();
		glPopAttrib();
		return;
	    }
	    int cachesize = ConfigManager::getInt("value","Plugin.PanoViewLOD.CacheSize",256);
	    _cacheMap[context] = new sph_cache(cachesize);
	    _cacheMap[context]->set_debug(false);

	    _updateLock[context] = new OpenThreads::Mutex();
	}

	int time = 1;
	if(_modelMap[context])
	{
	    time = _modelMap[context]->get_time();
	    delete _modelMap[context];
	}
	GLint buffer,ebuffer;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&buffer);
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING,&ebuffer);

	_modelMap[context] = new sph_model(*_cacheMap[context],_vertData,_fragData,_mesh,_depth,_size);
	// Set start time to last model's time since timestamps are used by the disk cache
	_modelMap[context]->set_time(time);

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebuffer);

	_leftFileIDs[context] = std::vector<int>();
	_rightFileIDs[context] = std::vector<int>();
    }

    if(!(_initMap[context] & eye))
    {
	if(eye & DRAW_LEFT)
	{
	    for(int i = 0; i < _leftEyeFiles.size(); i++)
	    {
		_leftFileIDs[context].push_back(_cacheMap[context]->add_file(_leftEyeFiles[i]));
	    }
	    if(_leftEyeFiles.size() > 1 && _currentIndex < _leftFileIDs[context].size())
	    {
                int last, next;
                next = (_currentIndex+1) % _leftFileIDs[context].size();
                last = (_currentIndex-1+_leftFileIDs[context].size()) % _leftFileIDs[context].size();
		sph_cache::_diskCache->setLeftFiles(_leftFileIDs[context][last],_leftFileIDs[context][_currentIndex],_leftFileIDs[context][next]);
	    }
	    else if(_leftEyeFiles.size() == 1 && _currentIndex < _leftFileIDs[context].size())
	    {
		sph_cache::_diskCache->setLeftFiles(-1,_leftFileIDs[context][_currentIndex],-1);
	    }
	}
	else if(eye & DRAW_RIGHT)
	{
	    for(int i = 0; i < _rightEyeFiles.size(); i++)
	    {
		_rightFileIDs[context].push_back(_cacheMap[context]->add_file(_rightEyeFiles[i]));
	    }
	    if(_rightEyeFiles.size() > 1 && _currentIndex < _rightFileIDs[context].size())
	    {
                int last, next;
                next = (_currentIndex+1) % _rightFileIDs[context].size();
                last = (_currentIndex-1+_rightFileIDs[context].size()) % _rightFileIDs[context].size();
		sph_cache::_diskCache->setRightFiles(_rightFileIDs[context][last],_rightFileIDs[context][_currentIndex],_rightFileIDs[context][next]);
	    }
	    else if(_rightEyeFiles.size() == 1 && _currentIndex < _rightFileIDs[context].size())
	    {
		sph_cache::_diskCache->setRightFiles(-1,_rightFileIDs[context][_currentIndex],-1);
	    }
	}
	_initMap[context] |= eye;
    }

    _initLock.unlock();

    _updateLock[context]->lock();

    if(!_updateDoneMap[context])
    {
#ifdef PRINT_TIMING
	struct timeval ustart, uend;
	gettimeofday(&ustart,NULL);
#endif
	_cacheMap[context]->update(_modelMap[context]->tick());
	_updateDoneMap[context] = true;
#ifdef PRINT_TIMING
	gettimeofday(&uend,NULL);
	double utime = (uend.tv_sec - ustart.tv_sec) + ((uend.tv_usec - ustart.tv_usec)/1000000.0);
	std::cerr << "Context: " << context << " Update time: " << utime << std::endl;
#endif
    }

    _updateLock[context]->unlock(); 

    osg::Matrix modelview;

    modelview.makeScale(osg::Vec3(_radius,_radius,_radius));
    modelview = modelview * ri.getState()->getModelViewMatrix();
    
    int fileID[2];
    int pv[2];
    int pc = 0;
    int fc = 0;
    float fade = 0;
    if(eye & DRAW_LEFT)
    {
	if(_currentFadeTime == 0.0)
	{
	    fileID[0] = _leftFileIDs[context][_currentIndex];
	    fc = 1;
	}
	else
	{
	    fileID[0] = _leftFileIDs[context][_lastIndex];
	    fileID[1] = _leftFileIDs[context][_currentIndex];
	    fc = 2;
	    pv[0] = _leftFileIDs[context][_nextIndex];
	    pc = 1;
	    fade = 1.0 - (_currentFadeTime / _totalFadeTime);
            //std::cerr << "Files: " << fileID[0] << " " << fileID[1] << std::endl;
	}
    }
    else if(eye & DRAW_RIGHT)
    {
	if(_currentFadeTime == 0.0)
	{
	    fileID[0] = _rightFileIDs[context][_currentIndex];
	    fc = 1;
	}
	else
	{
	    fileID[0] = _rightFileIDs[context][_lastIndex];
	    fileID[1] = _rightFileIDs[context][_currentIndex];
	    fc = 2;
	    pv[0] = _rightFileIDs[context][_nextIndex];
	    pc = 1;
	    fade = 1.0 - (_currentFadeTime / _totalFadeTime);
            //std::cerr << "Files: " << fileID[0] << " " << fileID[1] << std::endl;
	}
    }

    //std::cerr << "Fade: " << fade << std::endl;

    _modelMap[context]->set_fade(fade);
    glUseProgram(0);
    _modelMap[context]->prep(ri.getState()->getProjectionMatrix().ptr(),modelview.ptr(), (int)ri.getState()->getCurrentViewport()->width(), (int)ri.getState()->getCurrentViewport()->height());

    //printGLState();
    MyGLClientState glstate;
    pushClientState(glstate);

    //GLint buffer,ebuffer;
    //glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&buffer);
    //glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING,&ebuffer);
    //bool vertexOn = glIsEnabled(GL_VERTEX_ARRAY);
    //glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    //clearGLClientState();

    _modelMap[context]->draw(ri.getState()->getProjectionMatrix().ptr(), modelview.ptr(), fileID, fc, pv, pc);

    popClientState(glstate);
    //glPopClientAttrib();
    //glBindBuffer(GL_ARRAY_BUFFER, buffer);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebuffer);
    /*if(vertexOn)
    {
	glEnableClientState(GL_VERTEX_ARRAY);
    }*/

    glPopAttrib();
}

void PanoDrawableLOD::PanoUpdate::update(osg::NodeVisitor *, osg::Drawable * drawable)
{
    PanoDrawableLOD * pdl = dynamic_cast<PanoDrawableLOD*>(drawable);
    if(!pdl)
    {
	return;
    }

    for(std::map<int,bool>::iterator it = pdl->_updateDoneMap.begin(); it != pdl->_updateDoneMap.end(); it++)
    {
	it->second = false;
    }

    if(pdl->_currentFadeTime > 0.0)
    {
	pdl->_currentFadeTime -= PluginHelper::getLastFrameDuration();
	if(pdl->_currentFadeTime < 0.0)
	{
	    pdl->_currentFadeTime = 0.0;
	}
    }
}
