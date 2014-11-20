{
  "targets": [
    {
      "target_name": "binding",
      "sources": [
        "src/binding.cc",
        "src/utils.cc",
        "src/py_object_wrapper.cc"
      ],
      "conditions": [
        ['OS=="mac"', {
            "xcode_settings": {
              "OTHER_CFLAGS": [
                "<!(python-config --cflags)"
              ],
              "OTHER_LDFLAGS": [
                "<!(python-config --ldflags)"
              ]
            }
        }, { # not OSX
          "cflags": [
            "<!(python-config --cflags)"
          ],
          'cflags': [ '-fexceptions', '-std=c++11' ],
          'cflags!': ['-fno-exceptions'],
          'cflags_cc': [ '-fexceptions' ],
          'cflags_cc!': [ '-fno-exceptions' ],
          "libraries": [
            "<!(python-config --libs)"
          ]
        }]
      ]
    }
  ]
}
