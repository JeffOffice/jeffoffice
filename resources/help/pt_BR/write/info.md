# Info: propriedades, senhas e Inspecionar

<!-- keywords: verificar acessibilidade, informações, propriedades, título, autor, assunto, palavras-chave, categorias, comentários, empresa, gerente, propriedades personalizadas, docproperty, marcar como final, final, somente leitura, senha, criptografar, proteger, senha para abrir, inspecionar documento, informações pessoais, texto oculto, XML personalizado, verificar compatibilidade, converter, modo de compatibilidade, histórico de versões, versões, manter uma versão, voltar, restaurar, comparar versões, mostrar na pasta, copiar caminho, pasta, caminho -->

Clique em **Arquivo > Informações** para ver os dados do seu documento e protegê-lo antes de compartilhá-lo. Pressione **Esc** para voltar ao documento.

## Onde está o arquivo

No alto, a página mostra onde o arquivo está salvo. Clique em **Mostrar na Pasta** para abrir essa pasta. Clique em **Copiar Caminho** para copiar o caminho e colá-lo em um e-mail ou em um chat.

## Propriedades

Propriedades são dados sobre o documento, como o título e quem o escreveu. A página tem uma caixa para cada uma: **Título**, **Autor**, **Assunto**, **Palavras-chave**, **Categorias**, **Comentários**, **Empresa** e **Gerente**. Digite em uma caixa e pressione **Tab** ou **Enter**. O JeffWrite mantém a alteração e a salva na próxima vez que você salvar o documento.

Abaixo das caixas, a página mostra o que o JeffWrite calcula: quando o documento foi criado, salvo e impresso, e quantas páginas, palavras, caracteres e parágrafos ele tem.

### Suas próprias propriedades

Clique em **Propriedades Personalizadas** para criar propriedades suas, como **Projeto** ou **Cliente**. Clique em **Adicionar**, digite um nome, escolha o tipo de valor (texto, número inteiro, número, sim ou não, ou data) e digite o valor.

Um campo pode mostrar uma propriedade personalizada. Clique em **Inserir > Campo** e escolha um campo. Altere o código para `DOCPROPERTY "Cliente"` e clique em **OK**. O texto mostra o valor. Pressione **F9** para atualizá-lo depois de alterar a propriedade. Veja [Campos](fields).

## Marcar como Final

Clique em **Marcar como Final** para informar aos leitores que o documento está concluído. O JeffWrite salva o documento e, em seguida, o mantém apenas para leitura. Quando alguém o abre, a barra de título indica que ele é somente leitura, e uma nota informa que ele foi marcado como final. É uma forma discreta de dizer "por favor, não altere isto". Não é um bloqueio, pois qualquer pessoa pode escolher **Marcada como Final** novamente para editá-lo.

## Sempre Abrir como Somente Leitura

Clique em **Sempre Abrir como Somente Leitura** para que o documento seja aberto apenas para leitura todas as vezes, evitando alterações acidentais. A escolha é salva com o arquivo quando você o salva. Na próxima vez que alguém abrir o arquivo, a barra de status informa que ele está aberto apenas para leitura. Para editá-lo, abra **Arquivo > Informações** e clique em **Sempre Abre como Somente Leitura**. Basta um clique, portanto é um lembrete e não um bloqueio. Outros programas não leem essa escolha, e um arquivo .odt não pode armazená-la, então salve primeiro o documento como arquivo .docx. A opção Verificar Compatibilidade a lista para um arquivo .odt.

## Criptografar com Senha

Clique em **Criptografar com Senha** para exigir uma senha sempre que o arquivo for aberto. Digite a senha duas vezes e clique em **Proteger**. O arquivo é embaralhado quando você o salva, de modo que ninguém pode lê-lo sem a senha. O JeffWrite usa o tipo forte de proteção que outros editores de texto usam, então eles também podem abrir o arquivo com a senha.

- Uma senha **não pode ser recuperada** se você a esquecer. Guarde-a em um lugar seguro.
- As versões que você guardou e a cópia de backup seguem a senha: elas são protegidas por ela e mudam quando você a altera ou remove.
- Para alterar a senha, clique novamente no mesmo cartão. Clique em **Remover Senha** para retirá-la. A alteração é salva na próxima vez que você salvar.
- Senhas funcionam para arquivos **.docx** e para arquivos **.odt**. Um arquivo .odt é protegido da maneira descrita pelo padrão OpenDocument, de modo que outros programas que seguem o padrão também podem abri-lo com a senha. Um arquivo de texto simples ou .rtf não pode armazenar senha, então o JeffWrite não salva um documento protegido nesses formatos. Em vez de salvar sem a senha, ele avisa você.

Para abrir um arquivo protegido, abra-o normalmente. O JeffWrite pede a senha. Se ela não estiver correta, você pode tentar novamente ou clicar em **Cancelar**.

O JeffWrite também abre um arquivo **.doc** antigo que pede senha. Ele pede da mesma forma. O JeffWrite nunca salva arquivos .doc, então **Salvar Como** pede um novo nome, e a cópia .docx mantém a mesma senha.

## Inspecionar Documento

Clique em **Inspecionar Documento** antes de enviar um documento para alguém. Ele lista o que o arquivo contém e que você talvez não queira compartilhar, e quantos itens de cada tipo foram encontrados:

- **Comentários** são notas que as pessoas deixaram na margem.
- **Alterações controladas** são edições que ainda não foram aceitas nem rejeitadas. **Aceitar Tudo** mantém cada alteração como parte do texto.
- **Propriedades do documento e informações pessoais** são o autor, a pessoa que salvou o arquivo por último, a empresa e o gerente.
- **Texto oculto** é texto configurado para ficar oculto. Ele não aparece nem é impresso, mas está no arquivo.
- **Cabeçalhos e rodapés** são os textos no alto e na base das páginas.
- **Dados XML personalizados** são dados que outros programas acrescentaram ao arquivo. Você não os vê em uma página.
- **Links externos** são links para outros arquivos, não links da web: uma imagem ou um objeto que está apenas vinculado (mostra um arquivo que não está no documento), e um campo que obtém seu texto ou imagem de outro arquivo (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE e DDEAUTO). **Remover Tudo** transforma cada campo no texto que ele mostra agora, retira cada imagem ou objeto vinculado e deixa o link fora do arquivo ao salvar. Os outros arquivos não são alterados. Links em cabeçalhos, rodapés, notas de rodapé e notas de fim também são contados e removidos. Links da web permanecem. Links dentro de comentários, caixas de texto e das partes que o programa mantém como estão (como o texto de desenhos) também são verificados. Uma imagem vinculada que também tenha sua própria cópia da imagem permanece, como uma imagem sem o link, e o link fica fora do arquivo ao salvar. Remover links dentro de comentários e das outras partes mantidas como estão não pode ser desfeito; as caixas de texto são uma etapa do Desfazer. O modelo ao qual o documento está anexado (seu caminho costuma conter um nome de usuário) também conta como link externo: **Remover Tudo** o desvincula, e o documento mantém os próprios estilos. Outros links para arquivos não são retirados, e uma linha abaixo da contagem os nomeia: a fonte de dados da mala direta, quadros, subdocumentos dentro de outras partes, dados externos de um gráfico, filmes e sons vinculados e folhas de estilo vinculadas. A mesma linha conta qualquer outro link em um campo, comentário, caixa de texto ou na lista de links do arquivo que nada no documento usa, e informa quando o endereço de uma imagem vinculada não está no arquivo ou quando uma parte está em uma codificação que a verificação não consegue ler. Nesse caso, a linha mostra "Não verificado por completo" em vez de "Nenhum encontrado". Um **Remover Tudo** retira no máximo 100.000 links das partes mantidas; se restarem mais, a linha informa quantos, e você pode executar **Remover Tudo** novamente.
- **Imagens originais mantidas sob efeitos de imagem** são as imagens a partir das quais um efeito, como Desfoque ou Mosaico, foi criado. O arquivo guarda o original ao lado da imagem final, e quem receber o arquivo pode extraí-lo. **Remover Originais** mantém apenas a imagem final, e **Redefinir Imagem** não poderá mais restaurar o original.
- **Partes cortadas de imagens** são as partes que você recortou para fora da vista. Elas continuam na imagem, e quem receber o arquivo pode vê-las. **Remover Partes Cortadas** as elimina, e a página continua exatamente como antes. Uma imagem com efeito se torna primeiro a sua imagem final.

Clique em **Remover Tudo** ao lado de um tipo para retirá-lo. Você pode desfazer as alterações no texto com **Ctrl+Z**. Links retirados de comentários e de outras partes mantidas, bem como dados XML personalizados, não podem ser desfeitos. Salve o documento depois para manter a cópia limpa.

**Remover informações pessoais deste arquivo ao salvar**, abaixo dos cartões, faz isso toda vez que você salva, para qualquer arquivo, sem alterar o documento aberto. É a mesma caixa das **Opções**, e a mesma em todos os aplicativos do JeffOffice.

## Converter

Um arquivo .docx que diz ter sido criado segundo a forma de um programa mais antigo mostra **[Modo de Compatibilidade]** após o nome na barra de título. Isso significa que o documento é disposto segundo as regras desse programa antigo. Um documento novo, um arquivo que não informa isso, e um arquivo .odt, .rtf, .doc ou de texto não mostram nada. Abra **Arquivo > Informações** e clique em **Converter** para usar as regras mais recentes. O programa pergunta antes, e a ação não pode ser desfeita com Ctrl+Z. As páginas ficam certas logo de imediato, como ficarão quando você salvar o arquivo e abri-lo novamente. A conversão altera estas coisas:

- **Tabelas.** No modo antigo, o recuo de uma tabela é medido até o texto da primeira célula, de modo que a borda esquerda da tabela fica um pouco à esquerda dele. No modo mais recente, o recuo é medido até a borda da tabela, então tabelas com recuo se deslocam para a direita pela largura da margem da célula.
- **Quebras de página.** No modo antigo, um parágrafo que termina com quebra de página faz o parágrafo seguinte começar a nova página. No modo mais recente, o fim desse parágrafo vai para a nova página e deixa uma linha vazia no alto dela.
- **Alturas de linha.** Um arquivo criado no modo mais antigo pode exigir linhas de pontos inteiros. Após a conversão, as linhas têm sua altura natural, então linhas e páginas podem quebrar em outros lugares.

Na próxima vez que você salvar o arquivo, ele informa que está no modo mais recente, e a barra de título perde a marca. Escolhas que o arquivo define uma a uma, como a hifenização e as páginas espelhadas, permanecem como estão. Um documento que já está no modo mais recente não tem o botão **Converter**.

## Verificar Compatibilidade

Clique em **Verificar Compatibilidade** e escolha um tipo de arquivo para ver o que ele não pode armazenar. O JeffWrite salva arquivos **.docx**, **.odt**, **.rtf** e **.txt**. Um arquivo .docx armazena tudo o que está no seu documento. Os outros tipos não podem armazenar algumas coisas, e esta lista indica quais. Ela também informa as escolhas guardadas no próprio arquivo, Marcar como Final e Sempre Abrir como Somente Leitura, quando estiverem ativadas, e a senha, para um tipo que não pode armazená-la. Se nada for listado, nada se perde.

Quando você salva como arquivo .odt, o JeffWrite lista o que o arquivo não pode armazenar **antes** de gravar qualquer coisa. Você pode escolher **Salvar Mesmo Assim**, **Salvar como .docx** (o arquivo .docx é gravado ao lado dele, e nada se perde) ou **Cancelar**. Marque **Não perguntar novamente para este arquivo** e o JeffWrite deixa de perguntar sobre esse arquivo até você fechar o programa.

## Verificar Acessibilidade

Clique em **Verificar Acessibilidade** para abrir a lista de itens que podem ser difíceis de ler ou usar para algumas pessoas, como uma imagem sem descrição. É a mesma verificação de **Revisão > Verificar Acessibilidade**. Veja [Contar palavras, definir o idioma, verificar acessibilidade e pesquisar palavras](proofing-tools).

## Histórico de Versões

Clique em **Histórico de Versões** para guardar cópias nomeadas do documento no seu computador. Nada é enviado a lugar nenhum. Uma versão é uma cópia completa do documento como ele está naquele momento. Guarde uma antes de uma mudança grande, para poder voltar a ela. O documento precisa estar salvo primeiro, porque as versões são guardadas com o nome do arquivo.

- **Manter uma Versão** pede um nome, como "Antes da edição", e guarda uma cópia.
- **Abrir uma Cópia** abre a versão escolhida em uma nova janela. A própria versão não muda.
- **Comparar** coloca o documento e a versão escolhida lado a lado, como um único documento, com as diferenças mostradas como alterações controladas. Veja [Controlar alterações](track-changes).
- **Voltar a Esta Versão** torna o documento igual à versão escolhida. O JeffWrite guarda primeiro o documento como ele estava, como uma nova versão, de modo que nada se perde. Salve o documento para manter a alteração.
- **Excluir** remove a versão escolhida.

O JeffWrite guarda até 50 versões de um arquivo e descarta as mais antigas quando há mais do que isso.
