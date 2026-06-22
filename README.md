<h2>Description</h2>
<p>
A simple, POSIX-compliant program to list information about files in a given directory (the current directory by default) inspired by ls.
</p>

<h2>Options Supported</h2>
<p>
<ol>
    <li>&nbsp;-a&emsp;&emsp;also show hidden files and 'dot' files</li>
    <li>&nbsp;-g&emsp;&emsp;same as -l, except the owner will not be written</li>
    <li>&nbsp;-i&emsp;&emsp; list each file's inode number</li>
    <li>&nbsp;-l&emsp;&emsp; use the long listing format</li>
    <li>&nbsp;-o&emsp;&emsp;same as -l, except the owning group will not be written</li>
</ol>
</p>

<h2>Fields Under The Long Listing Format</h2>
<p>
<ol>
    <li>File or directory protection</li>
    <li>Number of links to the file</li>
    <li>Owner's name</li>
    <li>Group's name</li>
    <li>Size of the file</li>
    <li>Date and time of last modification</li>
    <li>Filename</li>
</ol>
</p>

<h2>Acknowledgements</h2>
<p>
Thanks to <a href="https://github.com/mononeon">mononeon</a> for implementing multi-column output layout when printing by default and using -i flag as well as for helping me with testing this program.
</p>