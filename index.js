
/**
 * Module entry point.
 * Exports the public API modules.
 */

r('./lib/encoder');
r('./lib/decoder');
r('./lib/parser');

/**
 * Require a module and copy all it's exports onto the module's exports.
 */

function r (m) {
  var mod = require(m);
  for (var i in mod) {
    exports[i] = mod[i];
  }
}
