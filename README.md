<a href="http://embedonix.com"><img src="res/embedonix.jpg" width=64 height=64 align="right" /></a>
#WAV2MP3 Encoder
This is an experimental project for a command line tool to convert <b>wave</b> files into <b>mp3</b> format using <a href="http://lame.sourceforge.net" target="_blank">LAME MP3 Encoder</a> library.
<br />
#What it has
<ul>
<li>At the moment, only PCM formatted files are supported:</li>
<ul>
<li>MONO: u8, s16, u16</li>
<li>STEREO: u8, s16, u16</li>
</ul>
<li>Threaded encoding using pthreads, check <b>'posix'</b> branch.</li>
<li>A minimalistic approach to parse RIFF format is provided</li>
</ul>
#What to come
<ul>
<li>Support for Microsoft Windows</li>
</ul>
#Compiling
<ul>
<li>Currently, only linux is supported.</li>
<li>GCC 4.9 or higher</li>
<li>libmp3lame should be installed</li>
</ul>
#Warning
The code in this repository is still incomplete, please do not use it for any critical tasks until further notice!
<br /><br />
<p style="color: #FFCDFF">Feel free to ask questions!</p>