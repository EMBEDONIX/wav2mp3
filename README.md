<a href="http://embedonix.com"><img src="res/embedonix.jpg" width=64 height=64 align="right" /></a>
#WAV2MP3 Encoder
This is an experimental project for a command line tool to convert <b>wave</b> files into <b>mp3</b> format using <a href="http://lame.sourceforge.net" target="_blank">LAME MP3 Encoder</a> library.
<br /> <br />
<b>What is provided?</b>
<ul>
<li>Support for both GNU/Linux and Windows platforms.</li>
<li>At the moment, only PCM formatted files are supported:</li>
<ul>
<li>MONO: u8, s16, u16.</li>
<li>STEREO: u8, s16, u16.</li>
</ul>
<li>Threaded encoding using pthreads (Linux and Windows).</li>
<li>A minimalistic approach to parse RIFF format is included.</li>
</ul>
<b>Compile guide</b>
<ul>
<li>GNU/Linux systems</li>
    <ul>
        <li>GCC 4.9 or higher.</li>
        <li>Run CMake then make the resulting Makefile. Or, import the project with any ide that supports CMake (e.g. CLion) or after running CMake (gui or cli)
        import the project with IDE's supporting Makefile (e.g. Eclipse).</li>
        <li>Make sure your system has <b>libmp3lame-dev</b> installed.</li> 
    </ul>
<li>Windows</li>
    <ul>
        <li>The code has been tested with Visual Studio 2015.</li>
        <li>Pre compiled mp3lame and pthread are included (only Release versions).</li>
        <li>Using CMake (gui or cli) on Windows you can generate Visual Studio project out of source files.</li>
        <li>If you want Debug mode, you have to compile libmp3lame and pthread in debug mode yourself!</li>
    </ul>
</ul>
<b>What will come soon...</b>
<ul>
<li>Support for more WAV formats (IEEE Float, etc.).</li>
<li>Testing compilation with MinGW.</li>
</ul>