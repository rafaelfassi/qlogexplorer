# Generate new release

1. Update the version in `CMakeLists.txt` at project root directory
2. Run the python script `packaging/updateReleaseInfo.py` **(Requires ImageMagick)**
3. Add new release info in `packaging/linux/xdg/metainfo/qlogexplorer.appdata.xml`
4. Commit the changes with message: `Version <NewVersion>`
5. Create a tag: `git tag -a v<NewVersion> -m "Version <NewVersion>"`
6. Push the commit with the tag `git push origin master --follow-tags`
7. Open PR to flathub and winget with updated manifest
