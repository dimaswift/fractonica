cd ../
python fips build web

#!/bin/bash

SOURCE_DIR="webpage/"
DEST_DIR="../fips-deploy/fractonica/web/html"
EXCLUDE_FILE="shell.html"

# Create destination directory if it doesn't exist
if [ ! -d "$DEST_DIR" ]; then
  mkdir -p "$DEST_DIR"
  if [ $? -ne 0 ]; then
    echo "Error: Failed to create destination directory '$DEST_DIR'."
    exit 1
  fi
fi

echo "Copying files from '$SOURCE_DIR' to '$DEST_DIR', excluding '$EXCLUDE_FILE'..."

# Find all files in the source directory and copy them to the destination, excluding the specified file
find "$SOURCE_DIR" -maxdepth 1 -type f -print0 | while IFS= read -r -d $'\0' file; do
  BASENAME=$(basename "$file")
  if [ "$BASENAME" != "$EXCLUDE_FILE" ]; then
    cp "$file" "$DEST_DIR/"
    if [ $? -ne 0 ]; then
      echo "Warning: Failed to copy '$file'."
    else
      echo "Copied: $BASENAME"
    fi
  else
    echo "Excluded: $BASENAME"
  fi
done
mv "../fips-deploy/fractonica/web/main.html" "../fips-deploy/fractonica/web/html/index.html"
mv "../fips-deploy/fractonica/web/main.wasm" "../fips-deploy/fractonica/web/html/main.wasm"
mv "../fips-deploy/fractonica/web/main.js" "../fips-deploy/fractonica/web/html/main.js"
echo "Copy process complete."
