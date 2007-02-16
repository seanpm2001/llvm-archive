from SCons.Environment import Environment as Environment
from SCons.Defaults import Mkdir
import re,fileinput,os,glob
from string import join as sjoin
from os.path import join as pjoin
from os.path import exists

def getTestCases(dir,env):
  context = pjoin(env['AbsSrcRoot'],'test',dir)
  result = []
  for d in glob.glob(pjoin(context,'*.*')):
    if not os.path.isdir(d):
      if not d == pjoin(context,'dg.exp'):
        result.append(d)
  return result

def SiteExpAction(target,source,env):
  tgtpath = pjoin(env['AbsObjRoot'],'test')
  if not exists(tgtpath):
    env.Execute(Mkdir(tgtpath))
  outf = open(pjoin(tgtpath,'site.exp'),"w")
  outf.write('## these variables are automatically generated by make ##\n')
  outf.write('# Do not edit here.  If you wish to override these values\n')
  outf.write('# edit the last section\n')
  outf.write('set target_triplet "fubar-pc-noos"\n')
  outf.write('set srcdir "' + env['AbsSrcRoot'] + '/test"\n')
  outf.write('set objdir "' + env['AbsObjRoot'] + '/test"\n')
  outf.write('set tmpdir "$objdir/tmp"\n')
  outf.write('set srcrootdir "' + env['AbsSrcRoot'] + '"\n')
  outf.write('set objrootdir "' + env['AbsObjRoot'] + '"\n')
  outf.write('set LLVM_lib "' + env['LLVM_lib'] + '"\n')
  outf.write('set LLVM_inc "' + env['LLVM_inc'] + '"\n')
  outf.write('set LLVM_bin "' + env['LLVM_bin'] + '"\n')
  outf.write('set LIBXML2_lib "' + env['LIBXML2_lib'] + '"\n')
  outf.write('set LIBXML2_inc "' + env['LIBXML2_inc'] + '"\n')
  outf.write('set APR_lib "' + env['APR_lib'] + '"\n')
  outf.write('set APR_inc "' + env['APR_inc'] + '"\n')
  outf.write('set APRU_lib "' + env['APRU_lib'] + '"\n')
  outf.write('set APRU_inc "' + env['APRU_inc'] + '"\n')
  outf.write('set llc "' + env['with_llc'] + '"\n')
  outf.write('set llvm_ld "' + env['with_llvm_ld'] + '"\n')
  outf.write('set gxx "' + env['with_gxx'] + '"\n')
  outf.write('## All vars above are generated by scons. Do Not Edit!\n')
  outf.close()
  return 0

def SiteExpMessage(target,source,env):
  return "Building DejaGnu site.exp file"

def CheckAction(target,source,env):
  if env['with_runtest'] == None:
    print "Testing was disabled because DejaGnu 'runtest' was not found"
    return 0

  context = os.path.basename(env.File(target[0]).path)
  context = re.sub('(.*?)\..*','\\1',context)
  os.system('cd ' + pjoin(env['BuildDir'],'test') +
      '; DEJAGNU="'+pjoin(env['AbsObjRoot'],'test','site.exp')+'" '+
      env['with_runtest'] + ' --tool ' + context)
  return 0

def CheckMessage(target,source,env):
  return "Running DejaGNU Test Suite"

def Check(env,dirs):
  checkAction = env.Action(CheckAction,CheckMessage)
  checkBuilder = env.Builder(action=checkAction,suffix='results')
  sitexpAction = env.Action(SiteExpAction,SiteExpMessage)
  sitexpBuilder = env.Builder(action=sitexpAction,suffix='exp')
  env.Append(BUILDERS = {'Check':checkBuilder,'SiteExp':sitexpBuilder})
  env.SiteExp('#test/site.exp',[])
  env.SetOption('num_jobs',1)
  for dir in dirs:
    env.Check(['#test/' + dir + '.sum','#test/' + dir + '.log'],
              getTestCases(dir,env)+['#test/site.exp'])
    env.Alias('check','#test/' + dir + '.log')
  return 1
