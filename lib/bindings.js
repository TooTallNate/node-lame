/**
 * Compat for changes from node 0.4.x to 0.6.x.
 */
try {
  module.exports = require('../build/Release/bindings');
} catch (e) { try {
  module.exports = require('../build/default/bindings');
} catch (e) {
  throw e;
}}
