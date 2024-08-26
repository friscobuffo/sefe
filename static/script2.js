function testSefe() {
    Module.ccall('sefeMainTest', null, [], []);
}

const sefeSvgOutputRed = document.getElementById('sefe-svg-output-red');
const sefeSvgOutputBlue = document.getElementById('sefe-svg-output-blue');

function visualizeTestSefe() {
    console.log("visualize sefe");
    const filenameBlue = '/embedding-blue.svg';
    const filenameRed = '/embedding-red.svg';
    try {
        const svgContentRed = FS.readFile(filenameRed, { encoding: 'utf8' });
        sefeSvgOutputRed.innerHTML = svgContentRed;
        const svgContentBlue = FS.readFile(filenameBlue, { encoding: 'utf8' });
        sefeSvgOutputBlue.innerHTML = svgContentBlue;
    } catch (e) {
        console.error('Error loading file:', e);
    }
}