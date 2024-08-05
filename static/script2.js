const fileInput = document.getElementById('fileInput');

fileInput.addEventListener('change', (event) => {
    const file = event.target.files[0];
    if (file) {
        const reader = new FileReader();
        reader.onload = function(e) {
            const contents = e.target.result;
            const filename = '/input.txt';
            FS.writeFile(filename, contents);
        };
        reader.readAsText(file);
    }
});