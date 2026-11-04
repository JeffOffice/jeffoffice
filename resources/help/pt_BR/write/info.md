# Info: propriedades, senhas e Inspecionar

<!-- keywords: info, propriedades, título, autor, assunto, palavras-chave, categorias, comentários, empresa, gerente, propriedades personalizadas, docproperty, marcar como final, final, somente leitura, senha, criptografar, proteger, senha de abertura, inspecionar documento, informações pessoais, texto oculto, xml personalizado, verificar compatibilidade, mostrar na pasta, copiar caminho, pasta, caminho -->

Clique em **Arquivo > Info** para ver os dados do seu documento e protegê-lo antes de compartilhá-lo. Pressione **Esc** para voltar ao documento.

## Onde o arquivo está

No topo, a página mostra onde o arquivo está salvo. Clique em **Mostrar na Pasta** para abrir essa pasta. Clique em **Copiar Caminho** para copiar o caminho e colá-lo em um email ou em um chat.

## Propriedades

Propriedades são informações sobre o documento, como o título e quem o escreveu. A página tem uma caixa para cada uma: **Título**, **Autor**, **Assunto**, **Palavras-chave**, **Categorias**, **Comentários**, **Empresa** e **Gerente**. Digite na caixa e pressione **Tab** ou **Enter**. O JeffWrite mantém a alteração e a salva na próxima vez que você salvar o documento.

Abaixo das caixas, a página mostra o que o JeffWrite calcula: quando o documento foi criado, salvo e impresso, e quantas páginas, palavras, caracteres e parágrafos ele tem.

### Suas próprias propriedades

Clique em **Propriedades Personalizadas** para criar propriedades próprias, como **Projeto** ou **Cliente**. Clique em **Adicionar**, digite um nome, escolha o tipo de valor (texto, número inteiro, número, sim ou não, ou data) e digite o valor.

Um campo pode mostrar uma propriedade personalizada. Clique em **Inserir > Campo** e escolha um campo. Altere o código para `DOCPROPERTY "Cliente"` e clique em **OK**. O texto mostra o valor. Pressione **F9** para atualizá-lo depois de alterar a propriedade. Veja [Campos](fields).

## Marcar como Final

Clique em **Marcar como Final** para informar aos leitores que o documento está concluído. O JeffWrite salva o documento e depois o mantém apenas para leitura. Quando alguém o abre, a barra de título indica que ele é somente leitura, e uma observação informa que ele foi marcado como final. Trata-se de uma forma gentil de pedir "por favor, não altere isto". Não é um bloqueio, porque qualquer pessoa pode escolher **Marcar como Final** novamente para editá-lo.

## Criptografar com Senha

Clique em **Criptografar com Senha** para exigir uma senha sempre que o arquivo for aberto. Digite a senha duas vezes e clique em **Proteger**. O arquivo é embaralhado ao ser salvo, de modo que ninguém consegue lê-lo sem a senha. O JeffWrite usa o tipo de proteção forte que outros processadores de texto utilizam, para que eles também possam abrir o arquivo com a senha.

- Uma senha **não pode ser recuperada** se você a esquecer. Guarde-a em um lugar seguro.
- Para alterar a senha, clique novamente no mesmo cartão. Clique em **Remover Senha** para retirá-la. A alteração é salva na próxima vez que você salvar.
- As senhas funcionam em arquivos **.docx**. Um arquivo **.odt** ainda não pode conter senha, por isso o JeffWrite não salva um documento protegido como .odt. Em vez de salvar sem a senha, ele avisa você.

Para abrir um arquivo protegido, abra-o normalmente. O JeffWrite pede a senha. Se ela não estiver correta, você pode tentar novamente ou clicar em **Cancelar**.

## Inspecionar Documento

Clique em **Inspecionar Documento** antes de enviar um documento a alguém. Ele lista o que o arquivo contém e que você talvez não queira compartilhar, e quantos itens de cada tipo foram encontrados:

- **Comentários** são notas que as pessoas deixaram na margem.
- **Alterações controladas** são edições que ainda não foram aceitas nem rejeitadas. **Aceitar Tudo** mantém cada alteração como parte do texto.
- **Propriedades do documento e informações pessoais** são o autor, a pessoa que salvou o documento por último, a empresa e o gerente.
- **Texto oculto** é texto definido como oculto. Ele não aparece nem é impresso, mas está no arquivo.
- **Cabeçalhos e rodapés** são o texto no topo e na base das páginas.
- **Dados XML personalizados** são dados que outros programas acrescentaram ao arquivo. Você não os vê em uma página.

Clique em **Remover Tudo** ao lado de um tipo para retirá-lo. Você pode desfazer as alterações no texto com **Ctrl+Z**. Depois, salve o documento para manter a cópia limpa.

## Verificar Compatibilidade

Clique em **Verificar Compatibilidade** para ver o que um tipo de arquivo mais antigo não consegue armazenar. O JeffWrite salva arquivos **.docx** e **.odt**. Um arquivo .docx armazena tudo o que há no seu documento. Um arquivo .odt não consegue armazenar algumas coisas, e esta lista indica quais. Se nada for listado, nada se perde.
