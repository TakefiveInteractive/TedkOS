var _state = require('./state');
var machine = _state.start;
var callExtendParse = _state.callExtendParse;
var callDefine = _state.callDefine;
var edgeTypes = require('./state').EdgeTypes;
var stateTypes = require('./state').StateTypes;
var stream = require('stream');
var fs = require('fs');
var process = require('process');

function convert(str, mach) {
	var ansStr = "";
	var currentByte = 0;
	var chunk;
	// previous state
	var prev = null;
	// current state
	var curr = mach;
    while (curr && (curr.size==0 || (currentByte + curr.size <= str.length))) {
    	chunk = str.substring(currentByte, currentByte + curr.size);
    	//console.log("state:", curr.name);
    	//console.log("reading", chunk? chunk.toString():"nul");
    	currentByte += curr.size;
    	if(curr.size)
    	{
    		var transferred = false;
    		var otherwise = null;
    		for(var i=0; i<curr.edges.length; i++)
    		{
    			var edge = curr.edges[i];
    			if(!edge.cond)
    			{
    				otherwise = edge;
    				continue;
    			}
    			if(edge.cond.test(chunk.toString()))
    			{
					//console.log("selected EDGE:", edge);
    				switch(edge.type)
    				{
    					case edgeTypes.consumePrint:
    					{
    						ansStr += chunk.toString();
    						break;
    					}
    					case edgeTypes.consumeSilent:
    						break;
						case edgeTypes.peek:
						{
							currentByte -= curr.size;
							break;
						}
    				}
    				prev = curr;
    				curr = edge.to;
    				transferred = true;
    				break;
    			}
    		}
    		if(!transferred && otherwise)
    		{
				//console.log("default EDGE:", edge);
				switch(otherwise.type)
				{
					case edgeTypes.consumePrint:
					{
						ansStr += chunk.toString();
						break;
					}
					case edgeTypes.consumeSilent:
						break;
					case edgeTypes.peek:
					{
						currentByte -= curr.size;
						break;
					}
				}
				prev = curr;
    			curr = otherwise.to;
    		}
    	}
    	else {
	    	//console.log("state:", curr);
    		if(curr.handler)
    		{
    			var change = curr.handler(str, currentByte, curr);
    			currentByte = change.newLocation;
    			prev = curr;
    			curr = change.newState;
    			ansStr += change.ansStr;
    		}
    		else if(curr.type == stateTypes.error)
    		{
    			console.log("Error near LINE: ", str.substring(0, currentByte - 1).split("\n").length);
    			var nextLine = str.indexOf('\n', currentByte);
    			var prevLine = str.lastIndexOf('\n', currentByte);
    			if(nextLine == -1)
    				nextLine = str.length;
    			if(prevLine == -1)
    				prevLine = 0;
    			console.log(str.substring(prevLine, nextLine));
    			if(prev)
	    			console.log("CAUSED BY: \n", prev);
    			process.exit();
    		}
    		else if(curr.edges.length > 0)
    		{
				prev = curr;
    			curr = curr.edges[0].to;
    		}
    		else
			{
				curr = null;
				console.log("Warning: possible early exit...");
			}
    	}
    }
    return ansStr;
}

var handlers = require('./handlers');

var macroConsts = {};

callDefine.handler = (str, loc, state) => {
	// temp eliminates "\\\n" endings
	var temp = str.substring(loc).replace(/\\\n/g," ");
	var define = temp;

	if(temp.indexOf('\n') != -1)
		define = temp.substring(0, temp.indexOf('\n'));

	try {
		eval(define.trim().replace(/[ \t]+/, "="));
	} catch(err) {}

	try {
		eval("macroConsts." + define.trim().replace(/[ \t]+/, "="));
	} catch(err) {}

	/*

	try {
		eval("macroConsts." + define.trim().replace(/[ \t]+/, "=").replace("=", "=macroConsts."));
	} catch(err) {}
	*/

	return {
		newLocation: loc,
		newState: _state.lineStart,
		ansStr: ""
	};
};

var content = fs.readFileSync('test').toString();
console.log( "Macros: ", macroConsts);
