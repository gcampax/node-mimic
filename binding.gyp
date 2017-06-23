{
  'targets': [{
    'target_name': 'mimic',
    'sources': [
      'src/mimic.cpp',
      'src/lang_list.cpp',
    ],
    'conditions': [
      ['OS=="linux"', {
        'defines': [
        ],
        'cflags': [
          '-Wall',
          '<!@(pkg-config --cflags mimic)',
        ],
        'ldflags': [
          '<!@(pkg-config  --libs-only-L --libs-only-other mimic)'
        ],
        'libraries': [
          '<!@(pkg-config  --libs-only-l --libs-only-other mimic)'
        ]
      }]
    ]
  }]
}
