# PlaynPlug
<strong>PlaynPlug</strong> is an educational audio plugin framework designed to help users learn audio programming in C++. 
Its primary focus is on audio processing, allowing users to write only the audio processing code while the rest of the necessary infrastructure is handled for them.

## Processor Hot Reloading
To support rapid prototyping, PlaynPlug enables hot reloading of audio processors at runtime. 
This eliminates the need to recompile the entire plugin. 
The plugin monitors changes in the linked processor and automatically reloads it when updates are detected, providing immediate feedback during development.

## Dynamic GUI Loading
The user interface is defined in a <code>Config.xml</code> file located within each course directory. 
When loading a course, the plugin dynamically loads the corresponding GUI.