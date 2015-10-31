module.exports = {
	// This json is original #extend json
	// Return the string you want to put into publish/

	/*	Usage
	 * #extend { "type": "table", "data": {
	 *  "11": "'c' ", "2": "'a' "
	 * }, "indent": 8, "group": 5, "size": 15,
	 *	"default": "'\0'"}
	 * #endextend
	 *
	 * indent unit: space
	 *
	 * (optional: indent=4, group=1)
	 *
	 * becomes
	 *
	 * ________'\0', '\0', 'a' , '\0', '\0', 
	 * ________'\0', '\0', '\0', '\0', '\0', 
	 * ________'\0', 'c' , '\0', '\0', '\0', 
	 */
	table: (json) => {
		var indent = json.indent ? json.indent : 4;
		var group = json.group ? json.group : 1;
		var size = json.size;
		var ans = "";
		var j = 0;
		for(var i = 0; i < size/group; i++) {
			ans += " ".repeat(indent);
			for(; j < i*group; j++)
			{
				var val;
				if(json.data[j])
					val = json.data[j];
				else if(json.data["\'" + String.fromCharCode(j) + "\'"])
					val = json.data["\'" + String.fromCharCode(j) + "\'"];
				else val = json.default;
				ans += val + ',';
			}
			ans += "\n";
		}
		if(j < size) {
			ans += " ".repeat(indent);
			for(; j < size; j++)
			{
				var val;
				if(json.data[j])
					val = json.data[val];
				else if(json.data[String.fromCharCode(j)])
					val = json.data[String.fromCharCode(j)];
				else val = json.default;
				ans += val + ',';
			}
			ans += "\n";
		}
		ans = ans.substring(0, ans.length-3) + "\n";
		return ans;
	}
}