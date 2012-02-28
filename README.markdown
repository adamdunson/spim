Congratulations, you've cloned the repo. Keep any general purpose notes in here.

Project Directory Structure
===========================

	bin/
		placeholder for compiled binaries (don't add these to the repo)
	doc/
		documentation (specifically, from webcourses)
	sample/
		test.asc and test_results.txt
	src/
		source code here (surprise, surprise!)


Coding Conventions
==================

* **Please use tabs for indentation!**
	+ I prefer tabs to spaces in C, Java, and other similar languages for the
	following reason:
		- My tabstop and shiftwidth are set to 2 spaces. If you prefer something
		else, it is trivial to adjust this: simply change the size of your tab
		character.
	+ Alternatively, we can mix and match tabs and spaces. :(
* I have no qualms on how you wrap single-line if/else statements, loops, etc.
	Just please be consistent, if you wouldn't mind.
* Don't overcomment, but if you write something and its meaning isn't
	immediately obvious, please put some comments above/beside it.


Git Instructions
================

Pulling, merging, resolving conflicts.
--------------------------------------
`git pull [origin] [branch]` will pull the latest HEAD of [branch] from
[origin] and try to merge it into your current working directory. If it
encounters a conflict -- meaning, you have modified a portion of the code that
it is trying to merge into -- it will tell you that you have conflicts and that
you need to correct them before it can continue.

Fixing merge conflicts is a fairly simple process.

1. Run `git diff` to see which files need to be fixed.
	* This will show you a list of files with conflicts and the location of the
		conflicts within them.
	* You will need to open and edit them one-by-one to fix them.
2. When you open a file, you will need to search for a line that looks like
	"<<< HEAD" (or similar).
	* This shows you the difference between your version (some hash string) and
		the version you're merging with (likely, HEAD).
	* Just replace the whole section (including the '<<<' lines) with the portion
		of code you'd like to keep.
3. Afterwards, save your changes and move on to the next conflict.

After fixing each merge conflict, run `git add [filename]` to add your fixes
for the conflict, and then `git commit` to commit the merge.

Make sure you run `git push [origin] [branch]` after fixing a merge.
