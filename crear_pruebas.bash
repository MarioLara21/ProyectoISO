# Crear directorios
mkdir -p test_dir/subdir1 test_dir/subdir2 test_dir/subdir3

# Archivos en el directorio principal
dd if=/dev/urandom of=test_dir/file1.txt bs=1M count=100
dd if=/dev/urandom of=test_dir/file2.jpg bs=1M count=150
dd if=/dev/urandom of=test_dir/file3.mp4 bs=1M count=200
dd if=/dev/urandom of=test_dir/file4.mp3 bs=1M count=50
dd if=/dev/urandom of=test_dir/file5.pdf bs=1M count=50

# Archivos en subdirectorios
dd if=/dev/urandom of=test_dir/subdir1/file1.docx bs=1M count=50
dd if=/dev/urandom of=test_dir/subdir1/file2.xlsx bs=1M count=100
dd if=/dev/urandom of=test_dir/subdir2/file1.zip bs=1M count=200
dd if=/dev/urandom of=test_dir/subdir2/file2.tar bs=1M count=50
dd if=/dev/urandom of=test_dir/subdir3/file1.gif bs=1M count=50
dd if=/dev/urandom of=test_dir/subdir3/file2.pptx bs=1M count=50
