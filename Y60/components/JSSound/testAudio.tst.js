/* __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2005, ART+COM AG Berlin, Germany
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
*/

use("UnitTest.js");
plug("y60JSSound");
plug("ProcFunctions");

function SoundUnitTest() {
    this.Constructor(this, "SoundUnitTest");
}

SoundUnitTest.prototype.Constructor = function(obj, theName) {
    UnitTest.prototype.Constructor(obj, theName);

    obj.runNewAudioLibTest = function() {
        var myNoisyString = expandEnvironment("${Y60_NOISY_SOUND_TESTS}");
        msleep(250);
        ENSURE("obj.mySoundManager.running == true");
        ENSURE("obj.mySoundManager.soundcount == 0");
        if (myNoisyString != "") {
            obj.mySoundManager.volume = 0.5;
            msleep(250);
            ENSURE("obj.mySoundManager.volume == 0.5");
            obj.mySoundManager.fadeToVolume(1.0, 0.1);
            msleep(150);
            ENSURE("obj.mySoundManager.volume == 1.0");
        } else {
            obj.mySoundManager.volume = 0;
        }

        obj.mySound = new Sound("../../../../sound/testfiles/aussentuer.mp3");
        ENSURE("obj.mySoundManager.soundcount == 1");
        ENSURE("obj.mySound.time == 0");
        obj.mySound.volume = 1.0;
        msleep(2500);
        ENSURE("obj.mySound.volume == 1.0");

        ENSURE("!obj.mySound.playing");
        ENSURE("obj.mySound.src == '../../../../sound/testfiles/aussentuer.mp3'");
        ENSURE("obj.mySound.time == 0.0");
        ENSURE("obj.mySound.looping == false");
        ENSURE("Math.abs(obj.mySound.duration-1.6195) < 0.0001");

        DTITLE("Playing sound...");
        obj.mySound.play();
        ENSURE("obj.mySound.playing");
        msleep(1000);
        DTITLE("Pausing sound...");
        obj.mySound.pause();
        ENSURE("!obj.mySound.playing");
        msleep(200);
        DTITLE("Playing sound...");
        obj.mySound.play();
        ENSURE("obj.mySound.playing");
        msleep(1000);
        ENSURE("obj.mySound.time > 0");
        DTITLE("Seek to second 1");
        obj.mySound.seek(1);
        ENSURE("Math.abs(obj.mySound.time - 1) < 0.1");

        DTITLE("Seek relative minus 0.5 seconds");
        obj.mySound.seekRelative(-0.5);
        ENSURE("Math.abs(obj.mySound.time - 0.5) < 0.1");

        DTITLE("Volume fade...");
        obj.mySound.fadeToVolume(0.5, 0.1);
        msleep(250);
        ENSURE("Math.abs(obj.mySound.volume-0.5) < 0.001");
        DTITLE("Stopping sound...");
        obj.mySound.stop();

        ENSURE("!obj.mySound.playing");

        delete obj.mySound;
        gc();
        msleep(100);
        ENSURE("obj.mySoundManager.soundcount == 0");
        DPRINT("obj.mySoundManager.soundcount");

        obj.mySound = new Sound("../../../../sound/testfiles/aussentuer.mp3");
        obj.mySound.play();
        ENSURE("obj.mySoundManager.soundcount == 1");
        obj.mySoundManager.stopAll();
        delete obj.mySound;
        gc();
        msleep(100);
        ENSURE("obj.mySoundManager.soundcount == 0");
        DPRINT("obj.mySoundManager.soundcount");
    }

    obj.runLeakTest = function() {

        print("### INITIAL USAGE:" + getProcessMemoryUsage());

        obj.mySoundManager.volume == 1.0;

        const mySoundFile = "../../../../sound/testfiles/Plopp_2a.wav"
        var mySound = null;

        for (var i = 0; i < 20; ++i) {
            mySound = new Sound(mySoundFile, false, false);
            //mySound = new Sound(mySoundFile, false, false);
            mySound.play();
            //while (mySound.playing) {
                //msleep(100);
            //}
            mySound = null;

            var myUsage = getProcessMemoryUsage();
            if (i == 0) {
                obj.myStartUsage = myUsage;
                print("### START USAGE:" + obj.myStartUsage);
            }
            obj.myDiff = myUsage - obj.myStartUsage;
            print("### TEST " + i + " USAGE:" + myUsage + " DIFF:" + obj.myDiff + " SOUNDCOUNT:" + obj.mySoundManager.soundcount);
        }
        mySound = null;

        var myFiniUsage = getProcessMemoryUsage();
        obj.myDiff = myFiniUsage - obj.myStartUsage;
        print("### FINI USAGE:" + myFiniUsage + " DIFF:" + obj.myDiff + " SOUNDCOUNT:" + obj.mySoundManager.soundcount);

        msleep(1000);
        gc();
        msleep(1000);
        ENSURE("obj.mySoundManager.soundcount == 0");

        var myGCUsage = getProcessMemoryUsage();
        obj.myDiff = myGCUsage - obj.myStartUsage;
        print("### GC USAGE: " + myGCUsage + " DIFF:" + obj.myDiff);
        ENSURE("obj.myDiff < (obj.myStartUsage * 0.2)");
    }

    obj.run = function() {
        obj.mySoundManager = new SoundManager();
        this.runNewAudioLibTest();
        //this.runLeakTest();
    }
}

function main() {
    var myTestName = "testAudio.tst.js";
    var mySuite = new UnitTestSuite(myTestName);

    mySuite.addTest(new SoundUnitTest());
    mySuite.run();

    print(">> Finished test suite '" + myTestName + "', return status = " +
          mySuite.returnStatus() + "");
    return mySuite.returnStatus();
}

var rc = main();
if (rc != 0) {
    exit(5);
};
