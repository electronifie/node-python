{
  "targets": [
    {
      "target_name": "binding",
      "sources": [
        "src/binding.cc",
        "src/utils.cc",
        "src/py_object_wrapper.cc"
      ],
      "include_dir": [
        "../bondmath/.node-virtualenv/include/python2.7"
      ],
      "conditions": [
        ['OS=="mac"', {
            "xcode_settings": {
              "OTHER_CFLAGS": [
                "-I../../scratch/bondmath/.node-virtualenv/include/python2.7 -I../../scratch/bondmath/.node-virtualenv/include/python2.7 -fno-strict-aliasing -fno-common -dynamic -arch x86_64 -arch i386 -g -Os -pipe -fno-common -fno-strict-aliasing -fwrapv -DENABLE_DTRACE -DMACOSX -DNDEBUG -Wall -Wstrict-prototypes -Wshorten-64-to-32 -DNDEBUG -g -fwrapv -Os -Wall -Wstrict-prototypes -DENABLE_DTRACE"
              ],
              "OTHER_LDFLAGS": [
                "-L../../scratch/bondmath/.node-virtualenv/lib/python2.7/config/ -ldl -framework CoreFoundation -lpython2.7"
              ]
            }
        }, { # not OSX
          "cflags": [
            "<!(python-config --cflags)"
          ],
          "libraries": [
            "<!(python-config --libs)"
          ]
        }]
      ]
    }
  ]
}
