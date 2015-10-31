
var EdgeTypes = {
    'peek': 0,
    'consumePrint': 1,
    'consumeSilent': 2
};

var StateTypes = {
    'normal': 0,
    'error': 1,
    'exit': 2
};

var start = {
    name : "start",
    type: StateTypes.normal,
    // how many characters are read.
    size: 0,
    edges: []
};

var lineStart = {
    name: "lineStart",
    type: StateTypes.normal,
    size: 1,
    edges: []
};

var macro = {
    name: "macro",
    type: StateTypes.normal,
    size: 8,
    edges: []
};

// NOT FINISHED: notProcessed
var notProcessed = {
    name: "notProcessed",
    type: StateTypes.normal,
    size: 1,
    edges: []
};

var cEscape = {
    name: "cEscape",
  type: StateTypes.normal,
  size: 1,
  edges: []
};

var cStartComment = {
    name: "cStartComment",
  type: StateTypes.normal,
  size: 1,
  edges: []
};

var cMultiComment = {
    name: "cMultiComment",
  type: StateTypes.normal,
  size: 1,
  edges: []
};

var cEndComment = {
    name: "cEndComment",
  type: StateTypes.normal,
  size: 1,
  edges: []
};

var error = {
    name: "error",
    type: StateTypes.error,
    size: 0,
    edges: []
};

var extendParseEntry = {
    name: "extendParseEntry",
    type: StateTypes.normal,
    size: 1,
    edges: []
};

var callExtendParse = {
    name: "callExtendParse",
    type: StateTypes.normal,
    handler: null,
    size: 0,
    edges: []
};

start.edges.push({
    to: lineStart,
    type: EdgeTypes.peek,
    cond: null
});

lineStart.edges.push({
    to: lineStart,
    type: EdgeTypes.consumePrint,
    cond: /^[\n\t ]$/
});

lineStart.edges.push({
    to: macro,
    type: EdgeTypes.peek,
    cond: /^#$/
});

lineStart.edges.push({
    to: notProcessed,
    type: EdgeTypes.consumePrint,
    // null <=> otherwise
    cond: null
});

macro.edges.push({
    to: extendParseEntry,
    type: EdgeTypes.consumeSilent,
    cond: /^#extend[ \t\n]$/
});

macro.edges.push({
    to: error,
    type: EdgeTypes.consumeSilent,
    cond: /^#extend{$/,
    desc: "Error! Expected space between #extend and {"
});

macro.edges.push({
    to: notProcessed,
    type: EdgeTypes.consumePrint,
    cond: null
});

extendParseEntry.edges.push({
    to: extendParseEntry,
    type: EdgeTypes.consumeSilent,
    cond: /^[\n\t ]$/
});

extendParseEntry.edges.push({
    to: callExtendParse,
    type: EdgeTypes.peek,
    cond: /^{$/
});

extendParseEntry.edges.push({
    to: error,
    type: EdgeTypes.peek,
    cond: null
});

notProcessed.edges.push({
    to: notProcessed,
    type: EdgeTypes.consumePrint,
    cond: null
});

notProcessed.edges.push({
    to: cEscape,
    type: EdgeTypes.consumePrint,
    cond: /^\\$/
});

notProcessed.edges.push({
    to: cStartComment,
    type: EdgeTypes.consumePrint,
    cond: /^\/$/
});

notProcessed.edges.push({
    to: lineStart,
    type: EdgeTypes.consumePrint,
    cond: /^\n$/
});

cEscape.edges.push({
    to: notProcessed,
    type: EdgeTypes.consumePrint,
    cond: null
});

cStartComment.edges.push({
    to: cMultiComment,
    type: EdgeTypes.consumePrint,
    cond: /^\*$/
});

cStartComment.edges.push({
    to: notProcessed,
    type: EdgeTypes.consumePrint,
    cond: null
});

cMultiComment.edges.push({
    to: cEndComment,
    type: EdgeTypes.consumePrint,
    cond: /^\*$/
});

cMultiComment.edges.push({
    to: cMultiComment,
    type: EdgeTypes.consumePrint,
    cond: null
});

cEndComment.edges.push({
    to: notProcessed,
    type: EdgeTypes.consumePrint,
    cond: /^\/$/
});

cEndComment.edges.push({
    to: cMultiComment,
    type: EdgeTypes.consumePrint,
    cond: null
});
/*
cMultiComment.edges.push({
    to: cMultiComment,
    type: EdgeTypes.consumePrint,
    cond: null
});

cMultiComment.edges.push({
    to: cMultiComment,
    type: EdgeTypes.consumePrint,
    cond: null
});
*/

module.exports = {start, EdgeTypes, StateTypes, callExtendParse, lineStart};