module.exports = function(grunt) {
  grunt.initConfig({
    bower: {
      install: {
         //just run $ grunt bower:install and you'll see files from your Bower packages in lib directory
        options: {
          cleanTargetDir: true,
          layout: 'byComponent',
        }
      }
    }
  });
  
  grunt.loadNpmTasks('grunt-bower-task');
}
