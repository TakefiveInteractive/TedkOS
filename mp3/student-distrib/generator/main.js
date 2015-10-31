var _state = require('./state');
var machine = _state.start;
var callExtendParse = _state.callExtendParse;
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

// Set handler DISPATCHER
callExtendParse.handler = (str, loc, state) => {
	var endTag = /\n[\t ]*#endextend/;
	var search = str.substring(loc).match(endTag);
	if(!search)
	{
		console.log("Error! Missing #endextend");
		console.log("Error near LINE: ", str.substring(0, loc - 1).split("\n").length);
		var nextLine = str.indexOf('\n', loc);
		var prevLine = str.lastIndexOf('\n', loc);
		console.log(str.substring(prevLine, nextLine));
		process.exit();
	}
	var jsonStr = str.substring(loc, loc + search.index);
	var ans = "";
	try {
		var input = JSON.parse(jsonStr);
		if(!input.type)
			throw ".type Missing";
		if(!handlers[input.type])
			throw ".type = " + input.type + " is not supported!"
		ans = handlers[input.type](input);
	}
	catch (err) {
		console.log("Error inside #extend:", err.toString());
		console.log("Error near LINE: ", str.substring(0, loc - 1).split("\n").length);
		var nextLine = str.indexOf('\n', loc);
		var prevLine = str.lastIndexOf('\n', loc);
		console.log(str.substring(prevLine, nextLine));
		process.exit();
	}
	return {
		newLocation: loc + search.index + search[0].length,
		newState: _state.lineStart,
		ansStr: ans
	};
};

var content = fs.readFileSync('test').toString();
console.log( "Process RESULT:\n", convert(content, machine));
