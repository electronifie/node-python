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
	      [ 'OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="solaris"', {
          "cflags": [
            "<!(python-config --cflags)",
             "-std=c++11",
             "-Xlinker -export-dynamic"
          ],
          "cflags!": [ "-fno-exceptions" ],
          "cflags_cc!": [ "-fno-exceptions" ],
          "libraries": [
            "<!(python-config --libs)"
          ]
        }],
        ['OS=="mac"', {
          "xcode_settings": {
            "OTHER_CFLAGS": [
              "<!(python-config --cflags)"
            ],
            "OTHER_LDFLAGS": [
              "<!(python-config --ldflags)"
            ]
          }
        }]
      ]
    }
  ]
}
