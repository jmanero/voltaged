var Queue = module.exports = function(options) {
  options = options || {};

  this.tasks = [];
  this.running = 0;
  this.concurrency = +(options.concurrency) || 1;
  this.paused = !!options.paused;
};

Queue.prototype.push = function(task, callback) {
  this.tasks.push([task, callback]);
  setImmediate(reactor.bind(this)); // Kick the work loop
};

Queue.prototype.pause = function() {
  this.paused = true;
};

Queue.prototype.resume = function() {
  this.paused = false;
  setImmediate(reactor.bind(this)); // Kick the work loop
};


function reactor() {
  var queue = this;
  if (this.paused) return;
  if (this.running >= this.concurrency) return;
  if (!this.tasks.length) return;

  var task = this.tasks[0][0];
  var callback = this.tasks[0][1];
  this.tasks.shift();

  this.running++;
  setImmediate(reactor.bind(this));

  try {
    task(function(err, data) {
      queue.running--;
      setImmediate(reactor.bind(queue));

      callback(err, data);
    });
  } catch (err) {
    setImmediate(reactor.bind(queue));
    callback(err);
  }
}
