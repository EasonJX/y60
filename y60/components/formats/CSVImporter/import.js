plug("CSV");

var _myArray = CSV.Importer.csv2array("simple.csv");

for (var i = 0, l = _myArray.length; i < l; ++i) {
    var myRow = "";
    for (var j = 0, lj = _myArray[i].length; j < lj; ++j) {
        myRow += _myArray[i][j] + " | ";
    }
    print(myRow + "\n");
}

