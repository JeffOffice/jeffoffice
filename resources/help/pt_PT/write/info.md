# Info: propriedades, palavras-passe e Inspecionar

<!-- keywords: verificar acessibilidade, informações, propriedades, título, autor, assunto, palavras-chave, categorias, comentários, empresa, gestor, propriedades personalizadas, docproperty, marcar como final, final, só de leitura, palavra-passe, encriptar, proteger, palavra-passe de abertura, inspecionar documento, informações pessoais, texto oculto, XML personalizado, verificar compatibilidade, histórico de versões, versões, guardar uma versão, voltar atrás, repor, comparar versões, mostrar na pasta, copiar caminho, pasta, caminho -->

Clique em **Ficheiro > Informações** para ver os dados do seu documento e para o proteger antes de o partilhar. Prima **Esc** para voltar ao documento.

## Onde está o ficheiro

No topo, a página mostra onde o ficheiro está guardado. Clique em **Mostrar na Pasta** para abrir essa pasta. Clique em **Copiar Caminho** para copiar o caminho e poder colá-lo num correio eletrónico ou numa conversa.

## Propriedades

As propriedades são dados sobre o documento, como o título e quem o escreveu. A página tem uma caixa para cada uma: **Título**, **Autor**, **Assunto**, **Palavras-chave**, **Categorias**, **Comentários**, **Empresa** e **Gestor**. Escreva numa caixa e prima **Tab** ou **Enter**. O JeffWrite mantém a alteração e guarda-a na próxima vez que guardar o documento.

Por baixo das caixas, a página mostra o que o JeffWrite calcula: quando o documento foi criado, guardado e impresso, e quantas páginas, palavras, carateres e parágrafos tem.

### As suas próprias propriedades

Clique em **Propriedades Personalizadas** para criar propriedades suas, como **Projeto** ou **Cliente**. Clique em **Adicionar**, escreva um nome, escolha o tipo de valor (texto, número inteiro, número, sim ou não, ou data) e escreva o valor.

Um campo pode mostrar uma propriedade personalizada. Clique em **Inserir > Campo** e escolha um campo. Altere o código para `DOCPROPERTY "Cliente"` e clique em **OK**. O texto mostra o valor. Prima **F9** para o atualizar depois de alterar a propriedade. Consulte [Campos](fields).

## Marcar como Final

Clique em **Marcar como Final** para indicar aos leitores que o documento está concluído. O JeffWrite guarda o documento e depois o mantém apenas para leitura. Quando alguém o abre, a barra de título indica que é só de leitura, e uma nota informa que foi marcado como final. Trata-se de uma forma suave de dizer «por favor, não altere isto». Não é um bloqueio, porque qualquer pessoa pode escolher **Marcado como Final** novamente para o editar.

## Abrir Sempre Só para Leitura

Clique em **Abrir Sempre Só para Leitura** para que o documento seja aberto apenas para leitura de cada vez, evitando que alguém o altere por engano. A escolha é guardada com o ficheiro quando o guardar. Na próxima vez que alguém abrir o ficheiro, a barra de estado indica que está aberto só para leitura. Para o editar, abra **Ficheiro > Informações** e clique em **Abre Sempre Só para Leitura**. É um único clique, por isso isto é um lembrete e não um bloqueio. Outros programas não leem esta escolha, e um ficheiro .odt não a pode guardar; por isso, guarde primeiro o documento como ficheiro .docx. Verificar Compatibilidade indica-a para um ficheiro .odt.

## Encriptar com Palavra-passe

Clique em **Encriptar com Palavra-passe** para pedir uma palavra-passe sempre que o ficheiro for aberto. Escreva a palavra-passe duas vezes e clique em **Proteger**. O ficheiro é codificado quando o guarda, para que ninguém o possa ler sem a palavra-passe. O JeffWrite usa o tipo de proteção forte que outros processadores de texto utilizam, para que também possam abrir o ficheiro com a palavra-passe.

- Uma palavra-passe **não pode ser recuperada** se a esquecer. Guarde-a num local seguro.
- As versões que guardou e a cópia de segurança seguem a palavra-passe: ficam protegidas com ela e mudam quando a alterar ou a remover.
- Para alterar a palavra-passe, clique novamente no mesmo cartão. Clique em **Remover Palavra-passe** para a retirar. A alteração é guardada na próxima vez que guardar.
- As palavras-passe funcionam para ficheiros **.docx** e para ficheiros **.odt**. Um ficheiro .odt é protegido da forma descrita pela norma OpenDocument, por isso outros programas que sigam a norma também o podem abrir com a palavra-passe. Um ficheiro de texto simples ou .rtf não pode guardar uma palavra-passe, pelo que o JeffWrite não guarda um documento protegido nesses formatos. Em vez de guardar sem a palavra-passe, avisa o utilizador.

Para abrir um ficheiro protegido, abra-o como de costume. O JeffWrite pede a palavra-passe. Se não estiver correta, pode tentar novamente ou clicar em **Cancelar**.

O JeffWrite também abre um ficheiro **.doc** antigo que peça uma palavra-passe. Pede-a da mesma forma. O JeffWrite nunca guarda ficheiros .doc, por isso **Guardar Como** pede um novo nome, e a cópia .docx mantém a mesma palavra-passe.

## Inspecionar Documento

Clique em **Inspecionar Documento** antes de enviar um documento a alguém. A ferramenta indica o que o ficheiro contém e que poderá não querer partilhar, e quantos elementos de cada tipo encontrou:

- **Comentários** são notas que as pessoas deixaram na margem.
- **Alterações controladas** são edições que ainda não foram aceites nem rejeitadas. **Aceitar Tudo** mantém cada alteração como parte do texto.
- **Propriedades do documento e informações pessoais** são o autor, a pessoa que o guardou por último, a empresa e o gestor.
- **Texto oculto** é texto definido como oculto. Não é apresentado nem impresso, mas está no ficheiro.
- **Cabeçalhos e rodapés** são o texto no topo e na base das páginas.
- **Dados XML personalizados** são dados que outros programas acrescentaram ao ficheiro. Não os pode ver numa página.
- **Ligações externas** são ligações a outros ficheiros, não ligações Web: uma imagem ou um objeto que está apenas ligado (mostra um ficheiro que não está no documento), e um campo que obtém o seu texto ou imagem de outro ficheiro (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE e DDEAUTO). **Remover Tudo** transforma cada campo no texto que mostra neste momento, retira cada imagem ou objeto ligado e deixa a ligação fora do ficheiro quando o guardar. Os outros ficheiros não são alterados. As ligações em cabeçalhos, rodapés, notas de rodapé e notas de fim também são contadas e removidas. As ligações Web mantêm-se. As ligações dentro de comentários, caixas de texto e das partes que o programa mantém como estão (como o texto de desenhos) também são procuradas. Uma imagem ligada que tenha também a sua própria cópia da imagem mantém-se, como imagem sem ligação, e a ligação fica fora do ficheiro quando o guardar. Remover ligações dentro de comentários e das outras partes mantidas como estão não pode ser anulado; as caixas de texto são um passo do Anular. O modelo a que o documento está associado (o seu caminho contém muitas vezes um nome de utilizador) também conta como ligação externa: **Remover Tudo** desassocia-o, e o documento mantém os seus próprios estilos. Outras ligações a ficheiros não são eliminadas, e uma linha abaixo da contagem indica-as: a origem de dados da impressão em série, molduras, subdocumentos dentro de outras partes, dados externos de um gráfico, filmes e sons ligados e folhas de estilo ligadas. A mesma linha conta qualquer outra ligação num campo, num comentário, numa caixa de texto ou na lista de ligações do ficheiro que nada no documento usa, e indica quando o endereço de uma imagem ligada não está no ficheiro ou quando uma parte está numa codificação que a verificação não consegue ler. Nesse caso, a linha diz «Não totalmente verificado» em vez de «Nenhum encontrado». Uma única **Remover Tudo** retira no máximo 100 000 ligações nas partes mantidas; se restarem mais, a linha indica quantas, e pode executar **Remover Tudo** novamente.
- **Imagens originais mantidas sob efeitos de imagem** são as imagens a partir das quais foi criado um efeito, como Desfocagem ou Mosaico. O ficheiro guarda o original ao lado da imagem final, e quem receber o ficheiro pode retirá-lo. **Remover Originais** mantém apenas a imagem final, e **Repor Imagem** já não consegue repor o original.
- **Partes recortadas de imagens** são as partes que cortou e que ficaram fora de vista. Continuam na imagem, e quem receber o ficheiro pode vê-las. **Remover Partes Recortadas** elimina-as, e a página fica exatamente como antes. Uma imagem com efeito passa primeiro a ser a sua imagem final.

Clique em **Remover Tudo**, ao lado de um tipo, para o retirar. Pode anular as alterações ao texto com **Ctrl+Z**. As ligações retiradas de comentários e de outras partes mantidas, bem como os dados XML personalizados, não podem ser anulados. Guarde o documento depois para manter a cópia limpa.

**Remover informações pessoais deste ficheiro ao guardar**, abaixo dos cartões, faz isto sempre que guarda, para qualquer ficheiro, sem alterar o documento que tem aberto. É a mesma caixa que está em **Opções**, e é igual em todas as aplicações JeffOffice.

## Verificar Compatibilidade

Clique em **Verificar Compatibilidade** e escolha um tipo de ficheiro para ver o que não consegue guardar. O JeffWrite guarda ficheiros **.docx**, **.odt**, **.rtf** e **.txt**. Um ficheiro .docx guarda tudo o que está no documento. Os outros tipos não conseguem guardar algumas coisas, e esta lista indica quais. Também indica as opções guardadas no próprio ficheiro, Marcar como Final e Abrir Sempre Só para Leitura, quando estiverem ativas, bem como a palavra-passe, para um tipo que não a consegue guardar. Se nada estiver indicado, nada se perde.

Ao guardar como ficheiro .odt, o JeffWrite indica o que o ficheiro não consegue guardar **antes** de escrever seja o que for. Pode escolher **Guardar Mesmo Assim**, **Guardar como .docx** (o ficheiro .docx é criado ao lado dele, e nada se perde) ou **Cancelar**. Marque **Não voltar a perguntar para este ficheiro** e o JeffWrite deixa de perguntar sobre esse ficheiro até fechar o programa.

## Verificar Acessibilidade

Clique em **Verificar Acessibilidade** para abrir a lista de coisas que podem ser difíceis de ler ou usar para algumas pessoas, como uma imagem sem descrição. É a mesma verificação que **Rever > Verificar Acessibilidade**. Consulte [Contar palavras, definir o idioma, verificar a acessibilidade e procurar palavras](proofing-tools).

## Histórico de Versões

Clique em **Histórico de Versões** para guardar cópias com nome do documento no seu computador. Nada é enviado para lado nenhum. Uma versão é uma cópia completa do documento tal como está naquele momento. Guarde uma antes de uma alteração grande, para poder voltar a ela. O documento tem de estar guardado primeiro, porque as versões são guardadas com o nome do ficheiro.

- **Guardar uma Versão** pede um nome, como «Antes da revisão», e guarda uma cópia.
- **Abrir uma Cópia** abre a versão escolhida numa nova janela. A própria versão não é alterada.
- **Comparar** coloca o documento e a versão escolhida lado a lado, como um único documento, com as diferenças apresentadas como alterações controladas. Consulte [Controlar alterações](track-changes).
- **Voltar a Esta Versão** torna o documento igual à versão escolhida. O JeffWrite guarda primeiro o documento tal como estava, como nova versão, para que nada se perca. Guarde o documento para manter a alteração.
- **Eliminar** remove a versão escolhida.

O JeffWrite guarda até 50 versões de um ficheiro e elimina as mais antigas quando houver mais do que essas.
