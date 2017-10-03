/*
  Copyright (C) 2016 by Wojciech Jaśkowski, Michał Kempka, Grzegorz Runc, Jakub Toczek, Marek Wydmuch

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "ViZDoomGamePython.h"
#include "ViZDoomController.h"
#include <unordered_set>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <vector>

std::vector<std::string> ENEMIES = {
        "Zombieman",
        "ShotgunGuy",
        "Archvile",
        "Revenant",
        "RevenantTracer",
        "Fatso",
        "ChaingunGuy",
        "DoomImp",
        "Demon",
        "Spectre",
        "Cacodemon",
        "BaronOfHell",
        "BaronBall",
        "HellKnight",
        "LostSoul",
        "SpiderMastermind",
        "Arachnotron",
        "Cyberdemon",
        "PainElemental",
        "WolfensteinSS",
        "CommanderKeen",
        "BossBrain",
        "BossEye",
        "BossTarget"
};


std::vector<std::string> DEFENSIVE = {
        "CustomMedikit",
        "GreenArmor",
        "BlueArmor",
        "HealthBonus",
        "ArmorBonus",
        "Stimpack",
        "Medikit",
        "InvulnerabilitySphere"
};


std::vector<std::string> AMMO = {
        "Clip",
        "ClipBox",
        "RocketAmmo",
        "RocketBox",
        "Cell",
        "CellPack",
        "Shell",
        "ShellBox",
        "Backpack"
};

std::vector<std::string> WEAPONS = {
        "BFG9000",
        "Chaingun",
        "Chainsaw",
        "RocketLauncher",
        "PlasmaRifle",
        "Shotgun",
        "SuperShotgun",
        "BFG"
};

std::vector<std::string> CORPSES = {
        "DeadCacodemon",
        "DeadMarine",
        "DeadZombieMan",
        "DeadDemon",
        "DeadLostSoul",
        "DeadDoomImp",
        "DeadShotgunGuy"
};

std::vector<std::string> PLAYERS = {
        "DoomPlayer"
};

std::vector<std::string> COURSE_CLASSES[] = {ENEMIES, DEFENSIVE, WEAPONS, AMMO};

uint8_t custom_label(const char* input){
        // puts(input);
        for (int i = 0; i < 4; i++){
                std::vector<std::string> myvect = COURSE_CLASSES[i];

		for (auto &a: myvect) {
			if (!strcmp(a.c_str(), input)) {
				return i+1;
			}
		}
        }
        return 0;
}
namespace vizdoom {

#define PY_NONE bpya::object()

#if PY_MAJOR_VERSION >= 3
        int
#else
        void
#endif
        init_numpy() {
                bpyn::array::set_module_and_type("numpy", "ndarray");
                import_array();
        }

        DoomGamePython::DoomGamePython() {
                init_numpy();
        }

        void DoomGamePython::setAction(bpy::list const &pyAction) {
                auto action = DoomGamePython::pyListToVector<double>(pyAction);
                ReleaseGIL gil = ReleaseGIL();
                DoomGame::setAction(action);
        }

        double DoomGamePython::makeAction(bpy::list const &pyAction, unsigned int tics) {
                auto action = DoomGamePython::pyListToVector<double>(pyAction);
                ReleaseGIL gil = ReleaseGIL();
                return DoomGame::makeAction(action, tics);
        }

        GameStatePython* DoomGamePython::getState() {
                if (this->state == nullptr) return nullptr;
//
//        TODO: the following line causes:
//        Fatal Python error: PyEval_SaveThread: NULL tstate
//        ReleaseGIL gil = ReleaseGIL();
                this->pyState = new GameStatePython();

                this->pyState->number = this->state->number;
                this->pyState->tic = this->state->tic;

                this->updateBuffersShapes();
                int colorDims = 3;
                if (this->getScreenChannels() == 1) colorDims = 2;

                if (this->state->screenBuffer != nullptr)
                        this->pyState->screenBuffer = this->dataToNumpyArray(colorDims, this->colorShape, NPY_UBYTE, this->state->screenBuffer->data());
                if (this->state->depthBuffer != nullptr)
                        this->pyState->depthBuffer = this->dataToNumpyArray(2, this->grayShape, NPY_UBYTE, this->state->depthBuffer->data());
                if (this->state->labelsBuffer != nullptr) {
                        /* Find the association for each value */
                        uint8_t seg[256];
                        memset(seg, 0, sizeof(uint8_t)*256);
                        if(this->state->labels.size() > 0){
                                for(auto i = this->state->labels.begin(); i != this->state->labels.end(); ++i) {
					seg[i->value] = custom_label(i->objectName.c_str());
                                }
                        }
                        uint8_t *d = this->state->labelsBuffer->data();
                        for (int i = 0; i < (this->grayShape[0]*this->grayShape[1]); i++) {
				d[i] = seg[d[i]];
                        }
                        this->pyState->labelsBuffer = this->dataToNumpyArray(2, this->grayShape, NPY_UBYTE, d);
                        this->pyState->realLabelsBuffer = this->dataToNumpyArray(2, this->grayShape, NPY_UBYTE, this->state->labelsBuffer->data());
                }
                if (this->state->automapBuffer != nullptr)
                        this->pyState->automapBuffer = this->dataToNumpyArray(colorDims, this->colorShape, NPY_UBYTE, this->state->automapBuffer->data());

                if (this->state->gameVariables.size() > 0) {
                        // Numpy array version
                        npy_intp shape = this->state->gameVariables.size();
                        this->pyState->gameVariables = dataToNumpyArray(1, &shape, NPY_DOUBLE, this->state->gameVariables.data());

                        // Python list version
                        //this->pyState->gameVariables = DoomGamePython::vectorToPyList<int>(this->state->gameVariables);
                }


                if(this->state->labels.size() > 0){
                        bpy::list pyLabels;
                        for(auto i = this->state->labels.begin(); i != this->state->labels.end(); ++i){
                                LabelPython pyLabel;
                                pyLabel.objectId = i->objectId;
                                pyLabel.objectName = bpy::str(i->objectName.c_str());
                                pyLabel.value = i->value;
                                pyLabel.objectPositionX = i->objectPositionX;
                                pyLabel.objectPositionY = i->objectPositionY;
                                pyLabel.objectPositionZ = i->objectPositionZ;
                                pyLabels.append(pyLabel);
                        }

                        this->pyState->labels = pyLabels;
                }

                return this->pyState;
        }

        bpy::list DoomGamePython::getLastAction() {
                return DoomGamePython::vectorToPyList(this->lastAction);
        }

        bpy::list DoomGamePython::getAvailableButtons(){
                return DoomGamePython::vectorToPyList(this->availableButtons);
        }

        void DoomGamePython::setAvailableButtons(bpy::list const &pyButtons){
                DoomGame::setAvailableButtons(DoomGamePython::pyListToVector<Button>(pyButtons));
        }

        bpy::list DoomGamePython::getAvailableGameVariables(){
                return DoomGamePython::vectorToPyList(this->availableGameVariables);
        }

        void DoomGamePython::setAvailableGameVariables(bpy::list const &pyGameVariables){
                DoomGame::setAvailableGameVariables(DoomGamePython::pyListToVector<GameVariable>(pyGameVariables));
        }

        // These functions are wrapped for manual GIL management
        void DoomGamePython::init(){
                ReleaseGIL gil = ReleaseGIL();
                DoomGame::init();
        }

        void DoomGamePython::advanceAction(unsigned int tics, bool updateState){
                ReleaseGIL gil = ReleaseGIL();
                DoomGame::advanceAction(tics, updateState);
        }

        void DoomGamePython::respawnPlayer(){
                ReleaseGIL gil = ReleaseGIL();
                DoomGame::respawnPlayer();
        }


        // These functions are workaround for
        // "TypeError: No registered converter was able to produce a C++ rvalue of type std::string from this Python object of type str"
        // on GCC versions lower then 5
        bool DoomGamePython::loadConfig(bpy::str const &pyPath){
                const char* cPath = bpy::extract<const char *>(pyPath);
                std::string path(cPath);
                ReleaseGIL gil = ReleaseGIL();
                return DoomGame::loadConfig(path);
        }

        void DoomGamePython::newEpisode(){
                ReleaseGIL gil = ReleaseGIL();
                DoomGame::newEpisode();
        }

        void DoomGamePython::newEpisode(bpy::str const &pyPath){
                const char* cPath = bpy::extract<const char *>(pyPath);
                std::string path(cPath);
                ReleaseGIL gil = ReleaseGIL();
                DoomGame::newEpisode(path);
        }

        void DoomGamePython::replayEpisode(bpy::str const &pyPath, unsigned int player){
                const char* cPath = bpy::extract<const char *>(pyPath);
                std::string path(cPath);
                ReleaseGIL gil = ReleaseGIL();
                DoomGame::replayEpisode(path, player);
        }

        void DoomGamePython::setViZDoomPath(bpy::str const &pyPath){
                const char* cPath = bpy::extract<const char *>(pyPath);
                std::string path(cPath);
                DoomGame::setViZDoomPath(path);
        }

        void DoomGamePython::setDoomGamePath(bpy::str const &pyPath){
                const char* cPath = bpy::extract<const char *>(pyPath);
                std::string path(cPath);
                DoomGame::setDoomGamePath(path);
        }

        void DoomGamePython::setDoomScenarioPath(bpy::str const &pyPath){
                const char* cPath = bpy::extract<const char *>(pyPath);
                std::string path(cPath);
                DoomGame::setDoomScenarioPath(path);
        }

        void DoomGamePython::setDoomMap(bpy::str const &pyMap){
                const char* cMap = bpy::extract<const char *>(pyMap);
                std::string map(cMap);
                DoomGame::setDoomMap(map);
        }

        void DoomGamePython::setDoomConfigPath(bpy::str const &pyPath){
                const char* cPath = bpy::extract<const char *>(pyPath);
                std::string path(cPath);
                DoomGame::setDoomConfigPath(path);
        }

        void DoomGamePython::addGameArgs(bpy::str const &pyArgs){
                const char* cArgs = bpy::extract<const char *>(pyArgs);
                std::string args(cArgs);
                DoomGame::addGameArgs(args);
        }

        void DoomGamePython::sendGameCommand(bpy::str const &pyCmd){
                const char* cCmd = bpy::extract<const char *>(pyCmd);
                std::string cmd(cCmd);
                DoomGame::sendGameCommand(cmd);
        }


        void DoomGamePython::updateBuffersShapes(){
                int channels = this->getScreenChannels();
                int width = this->getScreenWidth();
                int height = this->getScreenHeight();

                switch(this->getScreenFormat()){
                case CRCGCB:
                case CBCGCR:
                        this->colorShape[0] = channels;
                        this->colorShape[1] = height;
                        this->colorShape[2] = width;
                        break;

                default:
                        this->colorShape[0] = height;
                        this->colorShape[1] = width;
                        this->colorShape[2] = channels;
                }

                this->grayShape[0] = height;
                this->grayShape[1] = width;
        }


        template<class T> bpy::list DoomGamePython::vectorToPyList(const std::vector<T>& vector){
                bpy::list pyList;
                for (auto i : vector) pyList.append(i);
                return pyList;
        }

        template<class T> std::vector<T> DoomGamePython::pyListToVector(bpy::list const &pyList){
                size_t pyListLength = bpy::len(pyList);
                std::vector<T> vector = std::vector<T>(pyListLength);
                for (size_t i = 0; i < pyListLength; ++i) vector[i] = bpy::extract<T>(pyList[i]);
                return vector;
        }

        bpy::object DoomGamePython::dataToNumpyArray(int dims, npy_intp *shape, int type, void *data) {
                PyObject *pyArray = PyArray_SimpleNewFromData(dims, shape, type, data);
                /* This line makes a copy: */
                PyObject *pyArrayCopied = PyArray_FROM_OTF(pyArray, type, NPY_ARRAY_ENSURECOPY | NPY_ARRAY_ENSUREARRAY);
                /* And this line gets rid of the old object which caused a memory leak: */
                Py_DECREF(pyArray);

                bpy::handle<> numpyArrayBoostHandle = bpy::handle<>(pyArrayCopied);
                bpy::object boostNumpyArray = bpy::object(numpyArrayBoostHandle);
                /* This line caused occasional segfaults in python3 */
                //bpyn::array numpyArray = bpyn::array(numpyHandle);

                return boostNumpyArray;
        }
}
