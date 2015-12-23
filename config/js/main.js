(function() {
  loadSettings();
  submitHandler();
})();

function submitHandler() {
  var $submitButton = $('#submitButton');

  $submitButton.on('click', function() {
    console.log('Submit');

    var return_to = getQueryParam('return_to', 'pebblejs://close#');
    var settings = saveSettings();
    document.location = return_to + encodeURIComponent(settings);
  });
}

function loadSettings() {
  if (localStorage.settings) {
    var settings = JSON.parse(localStorage.settings);
    for (key in settings) {
      $("#" + key).val(settings[key]);
    }
    $('[type="range"]').each(function() {
      var $textEquiv = $('[type="text"][name="' + $(this).attr('name') + '"]');
      $(this).val($textEquiv.val());
    });
  }
}

function saveSettings() {
  var settings = {}
  // Add all textual values
  $('textarea, select, [type="number"], [type="text"]').each(function() {
    settings[$(this).attr('id')] = $(this).val();
  });
  // Add all checkbox type values
  $('[type="radio"], [type="checkbox"]').each(function() {
    settings[$(this).attr('id')] = $(this).is(':checked');
  });
  
  var savedSettings = JSON.stringify(settings);
  localStorage.settings = savedSettings;
  console.log('Got settings: ' + savedSettings);

  return savedSettings;
}

function getQueryParam(variable, defaultValue) {
  var query = location.search.substring(1);
  var vars = query.split('&');
  for (var i = 0; i < vars.length; i++) {
    var pair = vars[i].split('=');
    if (pair[0] === variable) {
      return decodeURIComponent(pair[1]);
    }
  }
  return defaultValue || false;
}
