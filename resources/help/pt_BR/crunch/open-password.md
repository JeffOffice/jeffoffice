# Abrir pastas de trabalho protegidas por senha
<!-- keywords: senha, protegida, protegido, criptografada, criptografia, abrir, bloqueada, senha incorreta, senha errada, cancelar, xlsx, xls, ods, proteção de arquivo, senha padrão, privado, confidencial -->

Algumas pastas de trabalho são protegidas por senha ao serem salvas: sem ela, nenhum programa consegue ler o conteúdo do arquivo. Isso é diferente de **Proteger Planilha** e **Proteger Pasta de Trabalho** (consulte [Proteger uma planilha ou a pasta de trabalho](protect)), que apenas impedem alterações. Uma pasta de trabalho protegida por senha não pode ser aberta de forma alguma até que a senha seja digitada. Esta página explica como abrir uma e como criar uma.

## Abrir uma

Abra o arquivo como qualquer outro: pressione **{key:Open}**, escolha o arquivo ou arraste-o para a janela. O JeffCrunch detecta que o arquivo está protegido e pede a senha antes de ler qualquer coisa:

1. Digite a senha na caixa **Este arquivo está protegido por senha. Senha:**. Os caracteres aparecem como pontos.
2. Clique em **Abrir**.

Se a senha não for a correta, o JeffCrunch avisa e pede novamente, quantas vezes você quiser. Clique em **Cancelar** para parar: nada é aberto, e as pastas de trabalho que você já tem abertas permanecem como estão. As senhas são exatas: letras maiúsculas, espaços e acentos contam.

O JeffCrunch não grava a senha em lugar nenhum. Ele a mantém na memória enquanto a pasta de trabalho está aberta, para que **Salvar** possa proteger o arquivo novamente, e a apaga quando você fecha a pasta de trabalho. Na próxima vez que você abrir o arquivo, ele pedirá a senha de novo.

## O que pode ser aberto

- **Pastas de trabalho .xlsx e .xlsm** protegidas de qualquer uma das duas formas que o programa original usa desde 2007: a mais recente (a chave tem 128, 192 ou 256 bits, e o JeffCrunch também verifica se ninguém alterou o arquivo depois que ele foi protegido) e a mais antiga (AES com chave de 128, 192 ou 256 bits).
- **Pastas de trabalho .xls** protegidas de qualquer uma das três formas desse formato: o embaralhamento simples dos arquivos mais antigos, o tipo de 40 bits e o tipo mais forte, que usa o serviço de chaves do próprio computador.
- **Planilhas .ods** protegidas por senha, nas três formas como o LibreOffice e o OpenOffice as gravaram: a mais recente (o LibreOffice 25 e versões posteriores bloqueiam o arquivo inteiro com uma chave de 256 bits e geram a chave a partir da senha com um método propositalmente lento e que usa muita memória, por isso abrir esse tipo de arquivo demora um pouco mais), o tipo de 256 bits das versões anteriores a essa e o tipo mais antigo.

Os arquivos .xls mais antigos têm limitações na própria senha. O embaralhamento simples dos arquivos mais antigos usa somente os primeiros 15 caracteres da senha e aceita apenas letras, dígitos e sinais do conjunto de caracteres ocidental (Latin-1): uma senha com outras letras (como gregas ou chinesas) nunca é reconhecida como correta nesse caso. O tipo de 40 bits aceita qualquer letra, mas somente os primeiros 16 caracteres. O JeffCrunch não grava arquivos .xls com senha.

Alguns arquivos são bloqueados com uma senha que todos conhecem, para que qualquer pessoa possa abri-los, mas não alterá-los por acidente. O JeffCrunch abre esses arquivos sem pedir senha.

## Quando um arquivo não pode ser aberto

- **"A senha está incorreta."** A senha digitada não é a mesma com que o arquivo foi protegido. Tente novamente com atenção às maiúsculas e aos espaços; o JeffCrunch não consegue recuperar uma senha perdida.
- **"Este arquivo está protegido de uma forma que este programa não consegue abrir."** O arquivo está bloqueado por algo diferente de uma senha (por exemplo, uma chave vinculada a um certificado) ou em um formato que o JeffCrunch não lê.
- **"A proteção por senha deste arquivo exige mais memória ou tempo do que este programa permite, por isso ele não foi aberto."** Um arquivo do LibreOffice 25 ou posterior define quanta memória e quanto tempo são necessários para verificar a senha. O JeffCrunch aceita até dez vezes o que o próprio LibreOffice define (640 MiB de memória), para que um arquivo feito para sobrecarregar seu computador não consiga fazer isso. Um arquivo gravado pelo próprio LibreOffice fica bem dentro desse limite.
- **"O arquivo está danificado ou foi alterado depois de ser protegido."** A senha estava correta, mas o conteúdo do arquivo não é o que foi bloqueado: ele foi truncado, danificado ou alterado por outro programa. Copie-o novamente a partir do original.

## Salvar com senha

Clique em **Arquivo** e em **Proteger com Senha** (ou em **Revisão > Proteger com Senha**), digite a senha duas vezes e clique em **Proteger**. A pasta de trabalho será gravada protegida na próxima vez que você a salvar: quem abrir o arquivo precisará digitar a senha. Para alterar a senha, escolha o mesmo comando e digite uma nova; para removê-la, clique em **Remover senha** e salve.

Uma pasta de trabalho que você abriu com senha a mantém enquanto estiver aberta, portanto **Salvar** grava o arquivo protegido novamente com a mesma senha. O JeffCrunch mantém a senha na memória somente até você fechar a pasta de trabalho e depois a apaga. O arquivo que você abriu com senha não fica desprotegido por acidente: se você salvar uma cópia como arquivo .xls ou .ods, que o JeffCrunch não consegue proteger, ele avisa antes que a cópia não será protegida, e você pode cancelar.

Use uma senha longa e guarde-a em um lugar seguro: ninguém, nem mesmo nós, consegue abrir o arquivo sem ela.
