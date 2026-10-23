# AcidSynth (acid bass synth, VST3)

Built in the cloud by GitHub, so you don't need Projucer, JUCE, or Visual Studio installed.

## Build it (about 10 minutes the first time)
1. Make a free account at github.com and create a new repository (any name, public or private).
2. Upload everything in this folder to it. On the repo page choose "Add file > Upload files"
   and drag in `CMakeLists.txt`, the `Source` folder and the `.github` folder.
   (If the hidden `.github` folder won't upload: "Add file > Create new file", type the name
   `.github/workflows/build.yml`, and paste in the contents of that file.)
3. Open the repo's **Actions** tab. The "Build AcidSynth" run starts by itself. Wait for the green tick.
4. Click the finished run, scroll to **Artifacts**, download **AcidSynth-Windows**, and unzip it.

## Install in FL Studio
1. Copy the `AcidSynth.vst3` folder into `C:\Program Files\Common Files\VST3\` (needs admin).
2. FL Studio: Options > Manage plugins > Find installed plugins (verify).
3. Add it from the Plugin database under Installed > Generators.
4. Play it from the piano roll. Overlap notes for slide; use high velocity (about 100+) for accent.

`AcidSynth.exe` (standalone) is also in the download if you want to test without FL Studio.

## If the build fails
Open the failed run, expand the red step, copy the error text and send it to me.
