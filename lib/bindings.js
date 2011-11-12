/**
 * Compat for changes from node 0.4.x to 0.6.x.
 */
try {
  module.exports = require('../build/Release/nodelame');
} catch (e) { try {
  module.exports = require('../build/default/nodelame');
} catch (e) {
  throw e;
}}
