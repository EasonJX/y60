//=============================================================================
// Copyright (C) 2003, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: create_documentation.js,v $
//   $Author: christian $
//   $Revision: 1.1 $
//   $Date: 2005/04/28 17:12:57 $
//
//
//=============================================================================

function SyntaxHighlighter() {
    this.Constructor(this);
}

SyntaxHighlighter.prototype.Constructor = function(Public) {

    Public.highlight = function(theCode) {
        var myResult = "";
        var myMatches = [];
    	for (var i = 0; i < myRegexList.length; ++i) {
    		myMatches = myMatches.concat(getMatches(theCode, myRegexList[i].regex, myRegexList[i].css));
    	}

    	if (myMatches.length) {
    	    var myPos = 0;
    	    for (var i = 0; i < myMatches.length; ++i) {
        		var myMatch = myMatches[i];
    		    myResult += addWord(theCode.substring(myPos, myMatch.index), null);
    		    myResult += addWord(myMatch.value, myMatch.css);
    		    myPos = myMatch.index + myMatch.value.length;
    	    }
    	    myResult += addWord(theCode.substring(myPos), null);
    	} else {
    	    myResult = addWord(theCode, null);
    	}

    	return myResult;
    }

	const myKeywords = 'abstract boolean break byte case catch char class const continue debugger ' +
					 'default delete do double else enum export extends false final finally float ' +
					 'for function goto if implements import in instanceof int interface long native ' +
					 'new null package private protected public return short static super switch ' +
					 'synchronized this throw throws transient true try typeof var void volatile while with';

	const myRegexList = [
		{ regex: new RegExp('//.*$', 'gm'),							css: 'comment' },			// one line comments
		{ regex: new RegExp('/\\*[\\s\\S]*?\\*/', 'g'),				css: 'comment' },			// multiline comments
		{ regex: new RegExp('"(?:[^"\n]|[\"])*?"', 'g'),			css: 'string' },			// double quoted strings
		{ regex: new RegExp("'(?:[^'\n]|[\'])*?'", 'g'),			css: 'string' },			// single quoted strings
		{ regex: new RegExp(getKeywords(myKeywords), 'gm'),		    css: 'keyword' }			// keywords
    ];

    function getKeywords(theString) {
    	return '\\b' + theString.replace(/ /g, '\\b|\\b') + '\\b';
    }

    // gets a list of all matches for a given regular expression
    function getMatches(theCode, theRegex, theCss) {
    	var myMatch = null;
        var myResult = [];
    	while((myMatch = theRegex.exec(theCode)) != null) {
    		myResult.push({value: myMatch[0], index: myMatch.index, css: theCss });
    	}
    	return myResult;
    }

    function addWord(theWord, theCSS) {
    	var myResult = null;

        // Create html entities
    	theWord = theWord.replace(/&/g, '&amp;');
    	theWord = theWord.replace(/ /g, '&nbsp;');
    	theWord = theWord.replace(/</g, '&lt;');
    	theWord = theWord.replace(/\n/gm, '&nbsp;<br>');

        // Add css class
    	if (theCSS) {
    		myResult = "<span class='" + theCSS + "'>" + theWord + "</span>";
    	} else {
    	    myResult = theWord;
    	}

    	return myResult;
    }
}