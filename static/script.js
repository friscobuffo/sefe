// const fileInput = document.getElementById('fileInput');

// fileInput.addEventListener('change', (event) => {
//     const file = event.target.files[0];
//     if (file) {
//         const reader = new FileReader();
//         reader.onload = function(e) {
//             const contents = e.target.result;
//             const filename = '/input.txt';
//             FS.writeFile(filename, contents);
//         };
//         reader.readAsText(file);
//     }
// });

// function embed() {
//     Module.ccall('embedLoadedFile', null, [], []);
// }

// const visualizeButton = document.getElementById('visualize');
// const svgOutput = document.getElementById('svgOutput');
// visualizeButton.addEventListener('click', () => {
//     const filename = '/embedding.svg';
//     try {
//         const svgContent = FS.readFile(filename, { encoding: 'utf8' });
//         svgOutput.innerHTML = svgContent;
//     } catch (e) {
//         console.error('Error loading file:', e);
//     }
// });

